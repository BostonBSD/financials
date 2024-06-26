# Financials 
Keep track of personal financial data such as gold/silver prices and stock prices.  Can also calculate RSI and fetch historical data via Yahoo!

<p float="left">
  <img src="/financials.png" height="45%" width="45%" >
  <img src="/financials2.png" height="45%" width="45%" > 
</p>

This application depends upon the following development packages to compile.
Their package name may vary depending upon the distro; libgtk3 instead of 
gtk3 for example:

gtk+-3.0 json-glib-1.0 glib-2.0 libcurl sqlite3 make llvm/gcc pkgconf

Once these development packages are installed, open a terminal to the project root directory.

Change to the src directory:

`cd src/`

Compile the project [this will make a build directory]:

`make`

Run the binary:

`../build/financials`

The binary can be copied to wherever the user binaries are installed [ usually `~/bin` `~/.bin` or `~/.local/bin`].

The user needs a free account at finnhub.io, which will give them an access key.

Finnhub's free account has a maximum of 60 API calls per minute.

If the software returns all zero values this means the API maximum has been reached, it is not a problem with the software.

Bullion data is collected through Yahoo! Finance [the Finnhub free account does not offer bullion data].

----------------------------------------------------------
Has been tested on FreeBSD 13.3, 
Debian “bookworm”, and Ubuntu 22.04; using amd64 architecture.

Has also been tested on FreeBSD 13.2 for arm64/aarch64.

Will likely run on any derivative of these operating systems 
[Linux Mint, Elementary OS, Trisquel, Raspberry PI OS, NomadBSD, GhostBSD, etc].

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

All other commodities and bond futures are valued similarly, bullion is more inelastic [there's always high demand]. 

Crypto will probably be similar also, although it is still developing a futures infrastructure, right now it is mostly supply and demand [not so very inelastic].

You want to hedge inflation [commodities, bonds, etc] before the central bank lowers rates.

You want to invest in cash [savings accounts, CDs, etc] before the central bank raises rates.

----------------------------------------------------------

<p float="center">
  <img src="/src/resources/Stocks-icon.png" height="15%" width="15%" > 
</p>

<a href="https://www.flaticon.com/free-icons/trends" title="Trends icons">Trends icon created by Freepik - Flaticon</a>

<a href="https://media.flaticon.com/license/license.pdf" title="Trends Icon License" target="_blank" rel="noopener noreferrer">Trends Icon License</a>
