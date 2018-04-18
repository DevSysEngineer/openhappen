<?php

namespace OpenHappen\Bot;

class Sitemap {

    protected $_request = NULL;
    protected $_sitemaps = [];

    public function __construct(string $url) {
        $this->_request = new Request($url);
    }

    public function retrieve() : array {
        /* Run request */
        list($status, $message) = $this->_request->retrieve();
        if (!$status) {
            return [FALSE, $message];
        }

        /* Convert to Smart DOM */
        list($smartDOM, $message) = $this->_request->convertToSmartDOM(SmartDOM::TYPE_SITEMAP);
        if ($smartDOM === NULL) {
            return [FALSE, $message];
        }

        /* Get some data */
        $this->_sitemaps = $smartDOM->getSitemaps();
        $this->_herfs = $smartDOM->getHrefs([ Request::TYPE_DOMAIN, Request::TYPE_PATH ]);

        /* Success */
        return [TRUE, ''];
    }
}
