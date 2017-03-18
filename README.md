# CacheHog - IPC via cache timing

Randomly accessing a big chunk memory causes a lot of main memory accesses due
to limited size of CPU caches. When two processes to that at the same time, a
noticeable slowdown can be observed.

This creates an interesting information channel. The transmission procedure is
as follows: Divide time into intervals (5 milliseconds, for example). If
transmitting a `1`, randomly access a chunk of memory ("hog the cache") during
the whole interval.  If transmitting a `0`, do nothing during the whole
interval.

To receive data, hog the cache and measure how fast it's happening (how many
operations per interval). Intuitively, "fast" means `0`, and "slow" means `1`
(because another process is transmitting).

There are several challenges:

1. The channel is very noisy (obviously - another processes are also running on
   the same system). To diminish noise effects, oversampling is used. One
   logical bit is transmitted over several intervals.

2. It's difficult to know when another process starts transmitting.

    Currently before transmission of a word (8 bits), a `SYNC` word is transmitted
    (`10101010`). On the receiving side it's detected using Hamming distance and
    used to align to bit boundaries of the sender.

3. We don't precisely know what is "slow" and "fast".

    The receiver measures operations per second for the duration of a whole word
    and then performs _thresholding_ - assigning the binary value `0` to a
    measurement if it's above average, and `1` if it's below average.

    TODO: This has significant drawbacks. For example, it's very difficult to
    recognize the words `00000000` and `11111111`. Maybe use the average from
    `SYNC` for thresholding the data?

## Running it

Compile with `make`.

The receiver reads from standard input, and the transmitter writes to standard
output. In one terminal run

    ./cachehog receive

Then in another:

    ./cachehog transmit

and type some text. It should appear (most probably garbled) at the receiving
side.

For example, transmitting this file could result in something like this
(hexdumped and interleaved with the original):

```
orig 00000000  23 20 43 61 63 68 65 48  6f 67 20 2d 20 49 50 43  |# CacheHog - IPC|
recv 00000000  23 20 43 61 63 68 65 48  6f 67 30 2d 20 49 50 43  |# CacheHog0- IPC|
orig 00000010  20 76 69 61 20 63 61 63  68 65 20 74 69 6d 69 6e  | via cache timin|
recv 00000010  20 76 9a 61 20 63 61 63  9a 65 20 74 69 6d 69 6e  | v.a cac.e timin|
orig 00000020  67 0a 0a 52 61 6e 64 6f  6d 6c 79 20 61 63 63 65  |g..Randomly acce|
recv 00000020  99 0a a0 a5 98 6e 99 6f  6d 6c 79 20 61 63 63 65  |.....n.omly acce|
orig 00000030  73 73 69 6e 67 20 61 20  62 69 67 20 63 68 75 6e  |ssing a big chun|
recv 00000030  73 73 69 6e 99 20 61 20  62 9a 67 20 63 68 75 6e  |ssin. a b.g chun|
orig 00000040  6b 20 6d 65 6d 6f 72 79  20 63 61 75 73 65 73 20  |k memory causes |
recv 00000040  9a 20 6d 65 6d 6f 72 9e  21 63 61 75 73 65 73 60  |. memor.!causes`|
orig 00000050  61 20 6c 6f 74 20 6f 66  20 6d 61 69 6e 20 6d 65  |a lot of main me|
recv 00000050  61 e0 6c 6f 74 20 6f 66  88 6d 61 69 6e 88 6d 65  |a.lot of.main.me|
orig 00000060  6d 6f 72 79 20 61 63 63  65 73 73 65 73 20 64 75  |mory accesses du|
recv 00000060  6d 6f 72 9e 20 61 63 63  65 73 73 65 73 20 64 75  |mor. accesses du|
orig 00000070  65 0a 74 6f 20 6c 69 6d  69 74 65 64 20 73 69 7a  |e.to limited siz|
recv 00000070  65 0a a7 6f 20 6c 69 6d  74 65 64 20 73 69 7a a6  |e..o limted siz.|
orig 00000080  65 20 6f 66 20 43 50 55  20 63 61 63 68 65 73 2e  |e of CPU caches.|
recv 00000080  20 6f 66 88 43 50 55 20  63 61 63 68 65 73 2e 88  | of.CPU caches..|
```

Not bad.

Note: Tested on my machine (i7-4710HQ), probably won't work well or at all on
others. If you experience issues, try randomly tweaking some parameters.

## Testbench

Compile with `make testbench`.

Run as:

    ./testbench [message]

If you don't specify a message, default one will be used.

Possible output:

    $ ./testbench
    Message size: 28
    Estimated running time: 22 sec
    Received 28 bytes (224 bits)
    Lost 0 bytes
    ORG: Lorem ipsum dolor sit amet.
    REC: Lorm ipsum dolorsit amet.
    Total error bits: 10, bit error rate: 0.044643

NOTE: testbench does not run any diffing algorithm so if any of byte is lost in
transmission, error bit count will be higher than you might expect.

NOTE2: unlike `./cachehog` testbench uses POSIX functions so will probably not
work on Windows (not tested!).

## Debugging

There are some `printf`s hidden behind the `DEBUG` flag. Look at the source.

## Ideas (feel free to implement)

- Add forward error correction (Hamming codes, or something else)
- Add acknowledgements and automatic retransmission on failure
- Use some fancy information theory to get better BER or transmission rate
- Collision detection and multiplexing
- Port to JavaScript and deanonymize unsuspecting Tor users
- Profit
