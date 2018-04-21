<?php

declare(strict_types=1);

namespace OpenHappen\Bot;

class Page extends Location {

    protected $_hashmarkHrefs = [];
    protected $_internalHrefs = [];
    protected $_externalHrefs = [];

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
        list($smartDOM, $message) = $this->_request->convertToSmartDOM();
        if ($smartDOM === NULL) {
            return [FALSE, $message];
        }

        /* Set some data */
        $this->_title = $smartDOM->getTitle();
        $this->_headers = $smartDOM->getHeaders();
        $this->_paragraphs = $smartDOM->getParagraphs();
        $this->_hashmarkHrefs = $smartDOM->getHrefs([ Request::TYPE_HASHMARK ]);

        /* Get request hos */
        $host = $this->_request->getHost();

        /* Get domains hrefs */
        $hrefs = $smartDOM->getHrefs([ Request::TYPE_DOMAIN, Request::TYPE_PATH ]);
        foreach ($hrefs as $href) {
            /* Check if href is internal */
            if ($href->getInternal($host)) {
                $this->_internalHrefs[] = $href;
            } else {
                $this->_externalHrefs[] = $href;
            }
        }

        /* Success */
        return [TRUE, ''];
    }

    public function getHashmarkHrefs() {
        return $this->_hashmarkHrefs;
    }

    public function getInternalHrefs() {
        return $this->_internalHrefs;
    }

    public function getExternalHrefs() {
        return $this->_externalHrefs;
    }
}
