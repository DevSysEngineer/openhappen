# OpenHappen
Openhappen is search website with no-ads or tracking scripts.

## Bot
For this project we have build our own web crawler. This web crawler inspect your website and retrieve important information for the search algorithm.
The bot based on web standards for crawling websites. The bot check always if the website has support for `robot.txt` or/and `sitemaps` beforing inspect the page. If the page is not allowed, the bot doen't inspect this page.

You can recognize the bot with this user agent:

```
Mozilla/5.0 (compatible; OpenHappenBot/0.1; +https://github.com/KvanSteijn/openhappen)
```

If you want for somereasion to block our bot, add the current lines below in your robots.txt file. It would be nice if you also create an `issue` why you blocking our bot. Maybe we can help each other?

```
User-agent: OpenHappenBot
Disallow: /
```


### Need help?
If you have great idea or found a bug in our bot? Please make issue or pull request on `GitHub`.
