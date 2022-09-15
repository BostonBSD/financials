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
which can be copied to wherever the user binaries are installed.

The user needs an account at finnhub.io, which will give them an access key.

Has been tested on FreeBSD 13.1 and Fedora 35.
