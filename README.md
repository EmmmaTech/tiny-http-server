# tiny http server

a little http server written completely in c, with the stdlib and bsd sockets!

my old draft can be found [here](https://gist.github.com/EmmmaTech/c0b6abb7a520e3bb917c1e41df6b6226), if so desired.

## features

- supports most requests made with `HTTP/1.1` (except for `CONNECT`)
- supports concurrent connections via multithreading
- handles http error responses
- supports custom handlers to process requests 
- tls support via openssl (off by default)

## usage

```
./build/httpsrv srvdir [port] [inaddr]
```

any file located in the srvdir will automatically be served by the file. any `/` path will automatically relocate to `index.html`.

handlers for http routes can be written using the `HANDLER_FUNC` & `HANDLER` macros. there is an example located in the `src/main.c` file.
