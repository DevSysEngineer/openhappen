<?php

namespace OpenHappen\Bot;

class Page {

    protected $_request = NULL;
    protected $_robots = NULL;
    protected $_smartDom = NULL;

    protected $_logs = [];

    protected $_hashmarkHrefs = [];
    protected $_internalHrefs = [];
    protected $_externalHrefs = [];

    public function __construct(string $url) {
        $this->_request = new Request($url);
        $this->_robots = new Robots($this->_request->getSimpleURL());
    }

    public function retrieve() : array {
        /* Run request */
        list($status, $message) = $this->_robots->retrieve();
        if ($status) {
            $path = $this->_request->getPath();
            if (!$this->_robots->isPathAllowed($path)) {
                return [FALSE, 'Blocked by robots.txt'];
            }
        } else {
            $this->_logs[] = 'Failed to retrieve robots.txt: ' . $message;
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

        /* Get request hos */
        $host = $this->_request->getHost();

        /* Set some data */
        $this->_hashmarkHrefs = $smartDOM->getHrefs([ Request::TYPE_HASHMARK ]);

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

    public function getLogs() {
        return $this->_logs;
    }

    public function getRequest() {
        return $this->_request;
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
