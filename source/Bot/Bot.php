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

    public function __construct($dataProvider) {
        $this->_dataProvider = $dataProvider;
    }

    protected function _checkHrefs(Request $request, array $hrefs) {
        /* Get URL's */
        $urls = [];
        foreach ($hrefs as $href) {
            $url = $href->getURL($request->getSimpleURL());
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

    public function log(string $text) {
        echo '[' . date('r') . '] ' . $text . PHP_EOL;
    }

    public function start(string $url, bool $deep = FALSE) : array {
        /* Create log log */
        $this->log('Processing ' . $url);

        /* Check if data provider is NULL */
        if ($this->_dataProvider === NULL) {
            return [NULL, 'Data provider is not valid'];
        }

        /* Create page object */
        $page = new Page($url);

        /* Retrieve page */
        $result = $page->retrieve();
        list($status, $message) = $result;
        if (!$status) {
            return [NULL, 'Failed to retieve page: ' . $message];
        }

        /* Add page */
        $this->_dataProvider->addPage($page);

        /* Check if deep is TRUE */
        if ($deep) {
            /* Create empty array for websites */
            $internalPages = [];

            /* Check if internal hrefs exists */
            $internalHrefs = $page->getInternalHrefs();
            $urls = $this->_checkHrefs($page->getRequest(), $internalHrefs);
            foreach ($urls as $url) {
                $result = $this->start($url, FALSE);
                list($childPage, $message) = $result;
                if ($childPage === NULL) {
                    $this->log($message);
                } else {
                    $internalPages[] = $childPage;
                }
            }

            /* Run deeper */
            foreach ($internalPages as $internalPage) {
                $internalHrefs = $internalPage->getInternalHrefs();
                $urls = $this->_checkHrefs($internalPage->getRequest(), $internalHrefs);
                foreach ($urls as $url) {
                    $result = $this->start($url, TRUE);
                    list($childPage, $message) = $result;
                    if ($childPage === NULL) {
                        $this->log($message);
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

/* Get data provider object */
switch ($dataProviderValue) {
    case 'json':
        $dataProviderObj = new DataProvider\JSON;
        break;
    default:
        exit('Unspported data provider' . PHP_EOL);
}

/* Create bot object */
$bot = new Bot($dataProviderObj);

/* Init bot */
list($status, $message) = $bot->init();
if (!$status) {
    exit($message . PHP_EOL);
}

do {
    /* Check if url value is not NULL */
    if (!empty($urlValue)) {
        list($status, $message) = $bot->start($urlValue, TRUE);
        if (!$status) {
            $bot->log($message);
        }
        $urlValue = NULL;
    }
} while (empty($urlValue));
