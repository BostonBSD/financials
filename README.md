# finnhub.io-stock-ticker
This is a very amature side project, proceed at your own risk!

Keep track of personal financial data such as gold/silver prices and stock prices.  Can also calculate RSI and fetch historical data via Yahoo!

This application depends upon the following packages:

gtk+-3.0 json-glib-1.0 glib-2.0 libcurl sqlite3 cmake

Once these packages are installed, open a terminal to the project root directory and make a build directory: 

mkdir build

and type: 

cd src

make

The resulting binary should be in the build directory,
which you can copy to wherever your user binaries are installed.

You need an account at finnhub.io, which will give you an access key.

I've tested builds on FreeBSD 13.1 and Fedora 36.
