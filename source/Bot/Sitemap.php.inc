<?php

namespace OpenHappen\Bot;

class Sitemap {

    protected $_request = NULL;

    public function __construct(string $url) {
        $this->_request = new Request($url);
    }

    public function retrieve() {

    }
}
