# finnhub.io-stock-ticker
This is a very amature side project, proceed at your own risk!

[ If you're the type that likes to sue people, be warned it will not work, 
  there is no liability nor warranty of any kind, you're better off talking 
  to a piece of paper than using this software!  Otherwise, if you're not the 
  type to sue, you may find it useful. ]

Keep track of personal financial data such as gold/silver prices and stock prices.  Can also calculate RSI and fetch historical data via Yahoo!

This application depends upon the following development packages to compile:

gtk+-3.0 json-glib-1.0 glib-2.0 libcurl sqlite3 cmake clang/gcc

Once these development packages are installed, open a terminal to the project root directory and make a build directory: 

mkdir build

and type: 

cd src

make

The resulting binary should be in the build directory,
which can be copied to wherever the user binaries are installed.

The user needs an account at finnhub.io, which will give them an access key.

Has been tested on FreeBSD 13.1 and Fedora 35.
