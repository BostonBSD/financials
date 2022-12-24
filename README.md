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

Once these development packages are installed, open a terminal to the project root directory.

Make a build directory and change to the src directory:

`mkdir build && cd src/`

Compile the project:

`make`

Run the binary:

`../build/financials`

The binary can be copied to wherever the user binaries are installed [ usually `~/bin` `~/.bin` or `~/.local/bin`].

The user needs a free account at finnhub.io, which will give them an access key.

Finnhub's free account has a maximum of 60 API calls per minute.

If the software returns all zero values this means the API maximum has been reached, it is not a problem with the software.

Bullion data is collected through Yahoo! Finance [the Finnhub free account does not offer bullion data].

----------------------------------------------------------
Has been tested on FreeBSD 13.1, 
Debian “bullseye”, and Ubuntu 22.04; using amd64 architecture.

----------------------------------------------------------

Some additional features could include cryptocurrency, api selection 
(inclusion of other data service providers), and RSI trendline calibration [we can account for trendlines by using one week intervals over a two year timespan instead of one day intervals].  

It might be useful to create a watch list that displays a list of stocks and their stats [one per row], the current RSI, whether it is an upward or downward trend, and an indicator string, updating maybe every 4 or 5 seconds [save the historical data on the first fetch operation].  

An RSS news reader that displays any current pertinent news in a plaintext format for listed stocks [Nasdaq RSS Feeds looks like a promising news source. GMarkup and Libxml2 look like useful libraries].

Conversion to Gtk4 after an XML file creation application is developed [they aren't developing glade any longer].

Other pattern indicators such as Williams %R, MACD, the moving average crossover, Commodity Channel Index, stochastic oscillator, etc.

In the distant future, or perhaps not at all, we could try to implement linear and probabilistic optimization for equity decision making [the lpsolve library and the GNU Linear Programming Kit look promising].

Migration to more Gnomish datatypes and style, etc.  I started this project to learn C [coming from a C++ background] and Gtk, now that we're entering beta mode, it's time to make it more canonical.  

I can also relicense my code under the GPL if anyone would like to fork this project [you could license it yourself under a GPL project, the BSD license allows for this; so long as the BSD license is complied with, but I also am willing to relicense it].

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
