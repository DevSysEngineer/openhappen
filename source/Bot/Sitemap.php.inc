<?php

declare(strict_types=1);

namespace OpenHappen\Bot;

class Sitemap extends Location {

    protected $_sitemapHrefs = [];
    protected $_hrefs = [];

    public function retrieve() : array {
        /* Call parent method */
        list($status, $message) = parent::retrieve();
        if (!$status) {
            return [FALSE, $message];
        }

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
        $this->_sitemapHrefs = $smartDOM->getSitemapHrefs([ Request::TYPE_DOMAIN, Request::TYPE_PATH ]);
        $this->_hrefs = $smartDOM->getHrefs([ Request::TYPE_DOMAIN, Request::TYPE_PATH ]);

        /* Success */
        return [TRUE, ''];
    }

    public function getSitemapHrefs() {
        return $this->_sitemapHrefs;
    }

    public function getHrefs() {
        return $this->_hrefs;
    }
}
