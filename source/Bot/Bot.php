<?php

namespace OpenHappen\Bot;

use OpenHappen\DataProvider;

/* Data providers */
require_once __DIR__ . '/../DataProvider/JSON.php.inc';

/* Bot scripts */
require_once __DIR__ . '/Href.php.inc';
require_once __DIR__ . '/SmartDOM.php.inc';
require_once __DIR__ . '/Request.php.inc';
require_once __DIR__ . '/Robots.php.inc';
require_once __DIR__ . '/Page.php.inc';

class Bot {

    protected $_dataProvider = NULL;
    protected $_domainExtensionCheck = FALSE;
    protected $_domainExtensions = [];

    public function __construct($dataProvider, array $domainExtensions = []) {
        $this->_dataProvider = $dataProvider;
        $this->_domainExtensionCheck = !empty($domainExtensions);
        $this->_domainExtensions = $domainExtensions;

    }

    protected function _log(string $text) {
        echo '[' . date('r') . '] ' . $text . PHP_EOL;
    }

    protected function _checkHrefs(Request $request, array $hrefs) {
        /* Get URL's */
        $urls = [];
        foreach ($hrefs as $href) {
            $url = $href->getURL($request->getDomainURL());
            if ($this->_dataProvider->retrievePage($url)) {
                $urls[] = $url;
            }
        }

        /* Return URL's */
        return $urls;
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
            /* Check if url value is not empty */
            if (!empty($url)) {
                list($status, $message) = $this->progress($url, TRUE);
                if (!$status) {
                    $this->_log($message);
                }
                $url = NULL;
            }
        } while (empty($url));
    }

    public function progress(string $url, bool $deep = FALSE, Robots $robots = NULL) : array {
        /* Create log log */
        $this->_log('Processing ' . $url);

        /* Check if data provider is NULL */
        if ($this->_dataProvider === NULL) {
            return [NULL, 'Data provider is not valid'];
        }

        /* Create page object */
        $page = new Page($url, $robots);

        /* Get request */
        $request = $page->getRequest();

        /* Check if domain extension check is enabled */
        if ($this->_domainExtensionCheck && !in_array($request->getExtension(), $this->_domainExtensions)) {
            return [NULL, 'Extension is different than the required extensions'];
        }

        /* Init page */
        list($status, $message) = $page->init();
        if (!$status) {
            return [NULL, 'Failed to init page: ' . $message];
        }

        /* Retrieve page */
        list($status, $message) = $page->retrieve();
        if (!$status) {
            return [NULL, 'Failed to retrieve page: ' . $message];
        }

        /* Add page */
        $this->_dataProvider->addPage($page);

        /* Check if deep is TRUE */
        if ($deep) {
            /* Create empty array for websites */
            $internalPages = [];

            /* Get robots object */
            $robots = $page->getRobots();

            /* Check if internal hrefs exists */
            $internalHrefs = $page->getInternalHrefs();
            $urls = $this->_checkHrefs($request, $internalHrefs);
            foreach ($urls as $url) {
                list($childPage, $message) = $this->progress($url, FALSE, $robots);
                if ($childPage === NULL) {
                    $this->_log($message);
                } else {
                    $internalPages[] = $childPage;
                }
            }

            /* Run deeper */
            foreach ($internalPages as $internalPage) {
                $internalHrefs = $internalPage->getInternalHrefs();
                $urls = $this->_checkHrefs($internalPage->getRequest(), $internalHrefs);
                foreach ($urls as $url) {
                    list($childPage, $message) = $this->progress($url, TRUE, $robots);
                    if ($childPage === NULL) {
                        $this->_log($message);
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
