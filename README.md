# OpenHappen
Openhappen is search website without advertisement and / or tracking scripts. We are not selling any data to other companies. OpenHappen is free project wich as with the aim that the end user can search without worries.

## Bot
For this project we have build our own web crawler. This web crawler inspect your website and retrieves important information for the search algorithm. The bot based on web standards for crawling websites. The bot always checks if the website has a `robots.txt` and / or `sitemaps` beforing inspect the page. If the page is not allowed, the bot doen't inspect the page.

You can recognize the bot by this user agent:

```
Mozilla/5.0 (compatible; OpenHappenBot/0.1; +https://github.com/KvanSteijn/openhappen)
```

### How to increase/decrease delay time for OpenHappen bot?
OpenHappen bot used default crawl delay of `10` seconds. If you want to change this value, you can add the lines below your  `robots.txt`. Change the word `xxx` to you prefer settings. The value is expressed in seconds. The max value is 20. Pleae note how higher the value in seconds, how longer it's takes to craw your website.

```
User-agent: OpenHappenBot
Crawl-delay: xxx
```

### How to block OpenHappen bot?
If you wish to block our bot, add the lines below in your `robots.txt`. It will be appreciated if you create an `issue` why you blocking our bot. Maybe we can help each other?

```
User-agent: OpenHappenBot
Disallow: /
```

### Need help?
If you have great idea or found a bug in our bot? Please make issue or pull request on `GitHub`.
