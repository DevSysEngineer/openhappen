<?php

namespace OpenHappen\Bot;

class Href {

    protected $_url = NULL;
    protected $_host = NULL;
    protected $_type = FALSE;

    public function __construct(string $url) {
        $object = Request::ParseURL($url);
        $this->_url = $object->url;
        $this->_host = $object->host;
        $this->_type = $object->type;
    }

    public function getType() : string {
        return $this->_type;
    }

    public function getURL(string $url) : string {
        switch ($this->_type) {
            case Request::TYPE_PATH:
                /* Remove backslash */
                if (substr($url, -1) === '/') {
                    $url = substr($url, 0, -1);
                }

                /* Return URL */
                return $url . $this->_url;
            case Request::TYPE_HASHMARK:
                return $url . $this->_url;
            default:
                return $this->_url;
        }
    }

    public function getInternal(string $host) : bool {
        return (in_array($this->_type, [Request::TYPE_PATH, Request::TYPE_HASHMARK]) || $this->_host === $host);
    }

    public function toStdClass(string $domainURL) : \stdClass {
        /* Create object */
        $object = new \stdClass;
        $object->url = $this->getURL($domainURL);

        /* Return object */
        return $object;
    }
}
