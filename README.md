# One-Way Chat

Compile with `make`.

Run receiver, then run sender to start sending messages.

```
Usage: ./receiver [PORT] [USERNAME]
```

```
Usage: ./sender [PORT] [ADDRESS] [USERNAME]
```

Fun fact: the basic behavior of this program can actually be replicated with netcat!

Example receiver:

```bash
nc -l 4444
```

Example sender:

```bash
# Assuming localhost
nc 127.0.0.1 4444
```

In addition, you can interchange the receiver/sender in the repository for a netcat one! Sort of neat, I think.
