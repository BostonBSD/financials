# finnhub.io-stock-ticker
Keep track of personal financial data such as gold/silver prices and stock prices.  Can also calculate RSI and fetch historical data via Yahoo!

<p float="left">
  <img src="/financials.png" height="45%" width="45%" >
  <img src="/financials2.png" height="45%" width="45%" > 
</p>

This application depends upon the following development packages to compile.
Their package name may vary depending upon the distro; libgtk3 instead of 
gtk3 for example:

gtk+-3.0 json-glib-1.0 glib-2.0 libcurl sqlite3 libx11 cmake clang/gcc

Once these development packages are installed, open a terminal to the project root directory and make a build directory: 

mkdir build

and type: 

cd src

make

The resulting binary should be in the build directory,
which can be copied to wherever the user binaries are installed.

The user needs a free account at finnhub.io, which will give them an access key.

Finnhub's free account has a maximum of 60 API calls per minute.

If the software returns all zero values this means the API maximum has been reached, it is not a problem with the software.

Bullion data is collected through Yahoo! Finance [the Finnhub free account does not offer bullion data].

----------------------------------------------------------
Has been tested on FreeBSD 13.1, Fedora 36, 
Debian “bullseye”, and Ubuntu 22.04.

----------------------------------------------------------

Some additional features could include cryptocurrency, api selection 
(inclusion of other data service providers), market indices, other metals, and RSI trendline calibration.

----------------------------------------------------------

RSI INDICATOR NOTE: 

The RSI indicator is a measure of speculative momentum and not anything intrinsic 
to the corporation, which also has an influence on the stock price.

Anomalies in the industry and in the corporation can create a false positive.  

On a rising trendline an RSI of 50 can be taken as an oversold signal.

On a falling trendline an RSI of 50 can be taken as an overbought signal.

The indicator is not calibrated for rising and falling trendlines.

Typically, an RSI less than 30 is oversold and an RSI greater than 70 is overbought.

The indicator can be used to make short term trade decisions, but is impractical for long term investing.

You want to buy when the stock is oversold.

You want to sell when the stock is overbought.  

----------------------------------------------------------

Note on bullion:

The gold / silver ratio determines which metal is over or undervalued relative to the other.
The ratio is not fixed and has no upper bound.

Typically 100 is a historic high and 50 is a historic low.

If the ratio is near 100 silver is relatively inexpensive.

If the ratio is near 50 gold is relatively inexpensive.

If you have already decided to invest in bullion, the ratio can be used to determine which metal to go long or short on.

Bullion rises with inflation, the spot price is based on futures trading, high/low current inflation does not mean high/low future inflation.

Central bank rates are an indication of future inflation [rising rates indicate lower future inflation, falling rates indicate higher future inflation].

The markets are very efficient; the decision to go long or short needs to be made before the central bank makes their decision.
[All other commodities are valued similarly, bullion is more inelastic. Crypto is probably similar, although still developing.]

----------------------------------------------------------

<p float="center">
  <img src="/src/resources/Stocks-icon.png" height="15%" width="15%" > 
</p>

<a href="https://www.flaticon.com/free-icons/trends" title="Trends icons">Trends icon created by Freepik - Flaticon</a>

<a href="https://media.flaticon.com/license/license.pdf" title="Trends Icon License" target="_blank" rel="noopener noreferrer">Trends Icon License</a>

