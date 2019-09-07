<?php

declare(strict_types=1);

namespace OpenHappen\Bot;

use OpenHappen\DataProvider;

/* Data providers */
require_once __DIR__ . '/../DataProvider/JSON.php.inc';

/* Bot scripts */
require_once __DIR__ . '/Href.php.inc';
require_once __DIR__ . '/SmartDOM.php.inc';
require_once __DIR__ . '/Request.php.inc';
require_once __DIR__ . '/Robots.php.inc';
require_once __DIR__ . '/Location.php.inc';
require_once __DIR__ . '/Sitemap.php.inc';
require_once __DIR__ . '/Page.php.inc';

class Bot {

    const TYPE_PAGE = 'page';
    const TYPE_SITEMAP = 'sitemap';

    protected $_version = 0;

    protected $_dataProvider = NULL;
    protected $_domainExtensionCheck = FALSE;
    protected $_domainExtensions = [];
    protected $_href = NULL;

    public function __construct($dataProvider, array $domainExtensions = []) {
        $this->_dataProvider = $dataProvider;
        $this->_domainExtensionCheck = !empty($domainExtensions);
        $this->_domainExtensions = $domainExtensions;

    }

    protected function _log(string $text) {
        echo '[' . date('r') . '][BOT] ' . $text . PHP_EOL;
    }

    public function init() : array {
        /* Init data provider */
        $result = $this->_dataProvider->init();
        list($status, $message) = $result;
        if (!$status) {
            $this->_dataProvider = NULL;
        }

        /* Return result */
        return $result;
    }

    public function start($url) {
        do {
            /* Check if url value is empty */
            if (empty($url)) {
                $url = NULL;
                continue;
            }

            $this->_href = new Href($url);
            try {
                /* Progress page */
                list($page, $message) = $this->progressPage($url, $this->_href, TRUE);
                if ($page === NULL) {
                    /* Write log */
                    $this->_log($message);

                    /* Stop here */
                    $url = NULL;
                    continue;
                }

                /* Get robots object */
                $robots = $page->getRobots();

                /* Get request and domain URL */
                $request = $page->getRequest();
                $domainURL = $request->getDomainURL();

                /* Get sitemaps from robots.txt */
                $sitemapHrefs = $robots->getSitemapHrefs();
                foreach ($sitemapHrefs as $sitemapHref) {
                    /* Get href url */
                    $url = $sitemapHref->getURL($domainURL);

                    /* Check if location already exists */
                    if ($this->_dataProvider->existsLocation($url)) {
                        /* Set Href information from robots.txt */
                        list($status, $message) = $this->_dataProvider->changeLocationValues($url, [
                            'nextExport' => $sitemapHref->getNextExport(),
                            'changeFrey' => $sitemapHref->getChangeFreq()
                        ]);

                        /* Check status */
                        if (!$status) {
                            $this->_log($message);
                            continue;
                        }
                    }

                    /* Check if location can be retrieved */
                    if ($this->_dataProvider->retrieveLocation($url)) {
                        list($status, $message) = $this->progressSitemap($url, $href, $robots);
                        if (!$status) {
                            $this->_log($message);
                        }
                    }
                }
            } catch (\Exception $e) {
                /* Failed */
                $this->_log($e);
            }

            /* Reset URL */
            $url = NULL;
        } while (empty($url));
    }

    public function progressSitemap(string $url, Href $href, Robots $robots) : array {
        /* Create log log */
        $this->_log('Processing sitemap ' . $url);

        /* Create page object */
        $sitemap = new Sitemap($url, $robots);

        /* Get request */
        $request = $sitemap->getRequest();
        $domainURL = $request->getDomainURL();

        /* Check if page url is not same as href url */
        if ($url !== $href->getURL($domainURL)) {
            return [NULL, 'Href url is different than sitemap url'];
        }

        /* Init page */
        list($status, $message) = $sitemap->init();
        if (!$status) {
            return [NULL, 'Failed to init sitemap: ' . $message];
        }

        /* Get robots object */
        $robots = $sitemap->getRobots();

        /* Check if crawl delay is higher than zero */
        $crawlDelay = $robots->getCrawlDelay();
        if ($crawlDelay > 0) {
            $this->_log('Crawl-delay found. Sleep for ' . $crawlDelay . ' seconds');
            sleep($crawlDelay);
        }

        /* Retrieve page */
        list($status, $message) = $sitemap->retrieve();
        if (!$status) {
            return [NULL, 'Failed to retrieve sitemap: ' . $message];
        }

        /* Add sitemap */
        $this->_dataProvider->addLocation($sitemap, $href);

        /* Check if hrefs exists */
        $hrefs = $sitemap->getHrefs();
        foreach ($hrefs as $href) {
            $url = $href->getURL($domainURL);
            if ($this->_dataProvider->existsLocation($url)) {
                if (!$this->_dataProvider->retrieveLocation($url)) {
                    /* Set Href information from robots.txt */
                    list($status, $message) = $this->_dataProvider->changeLocationValues($url, [
                        'nextExport' => $href->getNextExport(),
                        'changeFrey' => $href->getChangeFreq()
                    ]);

                    /* Check status */
                    if (!$status) {
                        $this->_log($message);
                    }
                } else {
                    /* Progress page with href information from the sitemap */
                    list($childPage, $message) = $this->progressPage($url, $href, FALSE, $robots);
                    if ($childPage === NULL) {
                        $this->_log($message);
                    }
                }
            } else {
                /* Progress page with href information from the sitemap */
                list($childPage, $message) = $this->progressPage($url, $href, FALSE, $robots);
                if ($childPage === NULL) {
                    $this->_log($message);
                }
            }
        }

        /* Get sitemap hrefs */
        $sitemapHrefs = $sitemap->getSitemapHrefs();
        foreach ($sitemapHrefs as $sitemapHref) {
            $url = $sitemapHref->getURL($domainURL);
            if ($this->_dataProvider->existsLocation($url)) {
                if (!$this->_dataProvider->retrieveLocation($url)) {
                    /* Set Href information from robots.txt */
                    list($status, $message) = $this->_dataProvider->changeLocationValues($url, [
                        'nextExport' => $sitemapHref->getNextExport(),
                        'changeFrey' => $sitemapHref->getChangeFreq()
                    ]);

                    /* Check status */
                    if (!$status) {
                        $this->_log($message);
                    }
                } else {
                    list($status, $message) = $this->progressSitemap($url, $sitemapHref, $robots);
                    if (!$status) {
                        $this->_log($message);
                    }
                }
            } else {
                /* Progress page with href information from the sitemap */
                list($status, $message) = $this->progressSitemap($url, $sitemapHref, $robots);
                if (!$status) {
                    $this->_log($message);
                }
            }
        }

        /* Success */
        return [TRUE, ''];
    }

    public function progressPage(string $url, Href $href, bool $deep = FALSE, Robots $robots = NULL) : array {
        /* Create log log */
        $this->_log('Processing page ' . $url);

        /* Check if data provider is NULL */
        if ($this->_dataProvider === NULL) {
            return [NULL, 'Data provider is not valid'];
        }

        /* Create page object */
        $page = new Page($url, $robots);

        /* Get request */
        $request = $page->getRequest();
        $domainURL = $request->getDomainURL();

        /* Check if page url is not same as href url */
        if ($url !== $href->getURL($domainURL)) {
            return [NULL, 'Href url is different than page url'];
        }

        /* Check if domain extension check is enabled */
        if ($this->_domainExtensionCheck && !in_array($request->getExtension(), $this->_domainExtensions)) {
            return [NULL, 'Extension is different than the required extensions'];
        }

        /* Before we are goging to init the page, we need to check if the page is not content */
        if (!$request->isContent()) {
            /* Create some log */
            $this->_log('Page is not content, try to connect with domain URL');

            /* Override current href */
            $this->_href = new Href($domainURL);

            /* Stop here, and try again with domain url */
            return $this->progressPage($domainURL, $this->_href, TRUE);
        }

        /* Init page */
        list($status, $message) = $page->init();
        if (!$status) {
            return [NULL, 'Failed to init page: ' . $message];
        }

        /* Get robots object */
        $robots = $page->getRobots();

        /* Check if crawl delay is higher than zero */
        $crawlDelay = $robots->getCrawlDelay();
        if ($crawlDelay > 0) {
            $this->_log('Crawl-delay found. Sleep for ' . $crawlDelay . ' seconds');
            sleep($crawlDelay);
        }

        /* Retrieve page */
        list($status, $message) = $page->retrieve();
        if (!$status) {
            return [NULL, 'Failed to retrieve page: ' . $message];
        }

        /* Add page */
        $this->_dataProvider->addLocation($page, $href);

        /* Check if deep is TRUE */
        if ($deep) {
            /* Create empty array */
            $internalPages = [];

            /* Check if internal hrefs exists */
            $internalHrefs = $page->getInternalHrefs();
            foreach ($internalHrefs as $internalHref) {
                $url = $internalHref->getURL($domainURL);
                if ($this->_dataProvider->retrieveLocation($url)) {
                    list($childPage, $message) = $this->progressPage($url, $internalHref, FALSE, $robots);
                    if ($childPage === NULL) {
                        $this->_log($message);
                    } else {
                        $internalPages[] = $childPage;
                    }
                }
            }

            /* Run deeper */
            foreach ($internalPages as $internalPage) {
                $internalHrefs = $internalPage->getInternalHrefs();
                foreach ($internalHrefs as $internalHref) {
                    /* Get domain URL */
                    $request = $internalPage->getRequest();
                    $domainURL = $request->getDomainURL();

                    /* Check if page already is retrieved */
                    $url = $internalHref->getURL($domainURL);
                    if ($this->_dataProvider->retrieveLocation($url)) {
                        list($childPage, $message) = $this->progressPage($url, $internalHref, TRUE, $robots);
                        if ($childPage === NULL) {
                            $this->_log($message);
                        }
                    }
                }
            }
        }

        /* Success */
        return [$page, ''];
    }
}

/* Check if you are running this script in CLI mode */
if (php_sapi_name() !== 'cli') {
    exit('Run only this script in CLI mode' . PHP_EOL);
}

/* Set some default info */
$urlValue = NULL;
$dataProviderValue = 'json';
$domainExtensionValues = [];

/* Get url path */
$urlKey = array_search('--url', $argv);
if ($urlKey !== FALSE && !empty($argv[$urlKey + 1])) {
    $urlValue = $argv[$urlKey + 1];
}

/* Get data provider */
$dataProviderKey = array_search('--data-provider', $argv);
if ($dataProviderKey !== FALSE && !empty($argv[$dataProviderKey + 1])) {
    $dataProviderValue = $argv[$dataProviderKey + 1];
}

/* Get domain extenions */
$domainExtensionsKey = array_search('--only-domain-extensions', $argv);
if ($domainExtensionsKey !== FALSE && !empty($argv[$domainExtensionsKey + 1])) {
    $domainExtensionValues = explode(',', $argv[$domainExtensionsKey + 1]);
}

/* Get data provider object */
switch ($dataProviderValue) {
    case 'json':
        $dataProviderObj = new DataProvider\JSON;
        break;
    default:
        exit('Unspported data provider' . PHP_EOL);
}

/* Create bot object */
$bot = new Bot($dataProviderObj, $domainExtensionValues);

/* Init bot */
list($status, $message) = $bot->init();
if (!$status) {
    exit($message . PHP_EOL);
}

/* Start bot */
$bot->start($urlValue);
