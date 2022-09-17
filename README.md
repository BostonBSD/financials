# finnhub.io-stock-ticker
Keep track of personal financial data such as gold/silver prices and stock prices.  Can also calculate RSI and fetch historical data via Yahoo!

<p float="left">
  <img src="/financials.png" height="45%" width="45%" >
  <img src="/financials2.png" height="45%" width="45%" > 
</p>

This application depends upon the following development packages to compile.
Their package name may vary depending upon the distro; libgtk3 instead of 
gtk3 for example:

gtk+-3.0 json-glib-1.0 glib-2.0 libcurl sqlite3 cmake clang/gcc

Once these development packages are installed, open a terminal to the project root directory and make a build directory: 

mkdir build

and type: 

cd src

make

The resulting binary should be in the build directory,
which can be copied to wherever the user binaries are installed.

The user needs an account at finnhub.io, which will give them an access key.

----------------------------------------------------------
Has been tested on FreeBSD 13.1, Fedora 35, 
the latest Debian release, and Ubuntu 22.04.

----------------------------------------------------------

Some additional features could include cryptocurrency, bullion stats, api selection 
(inclusion of other data service providers), and RSI trendline calibration.

----------------------------------------------------------

RSI INDICATOR NOTE: 

The RSI indicator is a measure of speculative fluctuation and not anything intrinsic 
to the corporation, which also has an influence on the stock price.

Anomalies in the industry and in the corporation can create a false positive.  

On a rising trendline an RSI of 50 can be taken as an Oversold Watch signal.

On a falling trendline an RSI of 50 can be taken as an Overbought Watch signal.

The indicator is not calibrated for rising and falling trendlines.

Typically, an RSI less than 30 is oversold, and an RSI greater than 70 is overbought.

The indicator can be used to make short term trade decisions, but is relatively 
useless for long term investing.
[If the RSI is overbought, you might not want to buy yet.  If the RSI is oversold you might
not want to sell yet.]
