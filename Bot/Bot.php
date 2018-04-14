<?php

namespace Bot;

use DataProvider;

/* Data providers */
require '../DataProvider/JSON.php.inc';

/* Bot scripts */
require 'Href.php.inc';
require 'SmartDOM.php.inc';
require 'Request.php.inc';
require 'Website.php.inc';

class Bot {

    protected $_dataProvider = NULL;

    public function __construct($dataProvider) {
        $this->_dataProvider = $dataProvider;
    }

    protected function _log(string $text) {
        echo '[' . date('r') . '] ' . $text . PHP_EOL;
    }

    protected function _checkHrefs(Request $request, array $hrefs) {
        /* Get URL's */
        $urls = [];
        foreach ($hrefs as $href) {
            $url = $href->getURL($request->getSimpleURL());
            if ($this->_dataProvider->retrieveWebsite($url)) {
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

    public function start(string $url, bool $deep = FALSE) : array {
        /* Create log log */
        self::_log('Processing ' . $url);

        /* Check if data provider is NULL */
        if ($this->_dataProvider === NULL) {
            return [NULL, 'Data provider is not valid'];
        }

        /* Create website object */
        $website = new Website($url);

        /* Retrieve website */
        $result = $website->retrieve();
        list($status, $message) = $result;
        if (!$status) {
            return [NULL, $message];
        }

        /* Add website */
        $this->_dataProvider->addWebsite($website);

        /* Check if deep is TRUE */
        if ($deep) {
            /* Create empty array for websites */
            $internalWebsites = [];

            /* Check if internal hrefs exists */
            $internalHrefs = $website->getInternalHrefs();
            $urls = $this->_checkHrefs($website->getRequest(), $internalHrefs);
            foreach ($urls as $url) {
                $result = $this->start($url, FALSE);
                list($childWebsite, $message) = $result;
                if ($childWebsite !== NULL) {
                    $internalWebsites[] = $childWebsite;
                } else {
                    return $result;
                }
            }

            /* Run deeper */
            foreach ($internalWebsites as $internalWebsite) {
                $internalHrefs = $internalWebsite->getInternalHrefs();
                $urls = $this->_checkHrefs($internalWebsite->getRequest(), $internalHrefs);
                foreach ($urls as $url) {
                    $result = $this->start($url, TRUE);
                    list($childWebsite, $message) = $result;
                    if ($childWebsite === NULL) {
                        return $result;
                    }
                }
            }
        }

        /* Success */
        return [$website, ''];
    }
}

/* Create data provider */
$dataProvider = new DataProvider\JSON;

/* Create bot object */
$bot = new Bot($dataProvider);

/* Init bot */
list($status, $message) = $bot->init();
if (!$status) {
    exit($message);
}

$url = 'https://nos.nl/';

/* Start bot */
$bot->start($url, TRUE);
