# OpenHappen
Openhappen is search website with no-ads or tracking scripts.

## Bot
For tthis project we have build our own web crawler. This web crawler inspect your website and retrieve important information for the search algorithm.
The bot based on web standards for crawling websites. The bot check always if the website has support for `robot.txt` or/and `sitemaps` beforing inspect the page. If the page is not allowed, the. bot will not inspect this page.

You can recognize the bot with this user agent:
`Mozilla/5.0 (compatible; OpenHappenBot/0.1; +https://github.com/KvanSteijn/openhappen)`
