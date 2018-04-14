<?php

namespace Bot;

class Request {

    const TYPE_DOMAIN = 'domain';
    const TYPE_OTHER = 'other';
    const TYPE_HASHMARK = 'hashmark';
    const TYPE_PATH = 'path';
    const TYPE_UNKNOWN = 'unknown';

    protected $_url = NULL;
    protected $_id = NULL;
    protected $_scheme  = NULL;
    protected $_host = NULL;
    protected $_domain = NULL;
    protected $_port = NULL;
    protected $_type = NULL;
    protected $_path = NULL;
    protected $_simpleURL = NULL;

    protected $_httpCode = 0;
    protected $_content = NULL;

    public function __construct(string $url) {
        $object = self::ParseURL($url);
        $this->_url = $object->url;
        $this->_id = $object->id;
        $this->_scheme = $object->scheme;
        $this->_host = $object->host;
        $this->_domain = $object->domain;
        $this->_port = $object->port;
        $this->_type = $object->type;
        $this->_path = $object->path;
        $this->_simpleURL = $object->simpleURL;
    }

    /**
     * Custom base64 encoding. Replace unsafe url chars
     *
     * @param string $url
     * @return string
     */
    static public function Base64URLEncode($url) : string {
        return strtr(base64_encode($url), '+/=', '._-');
    }

    /**
     * Custom base64 decode. Replace custom url safe values with normal
     * base64 characters before decoding.
     *
     * @param string $url
     * @return string
     */
    static public function Base64URLDecode($url) : string {
        return base64_decode(strtr($url, '._-', '+/='));
    }

    static public function ParseURL(string $url) {
        /* Create object */
        $object = new \stdClass;
        $object->url = strtolower($url);
        $object->id = self::Base64URLEncode($object->url);
        $object->scheme = NULL;
        $object->host = NULL;
        $object->port = NULL;
        $object->domain = NULL;
        $object->path = NULL;

        /* Check if url is hashmark */
        $resultHashmark = (substr($object->url, 0, 1) === '#');
        $resultPath = (substr($object->url, 0, 1) === '/');
        if ($resultHashmark || $resultPath) {
            /* Get delimiter */
            if ($resultHashmark) {
                $delimiter = '#';
                $validType = self::TYPE_HASHMARK;
            } elseif ($resultPath) {
                $delimiter = '/';
                $validType = self::TYPE_PATH;
            }

            /* Check hash mark */
            $explURL = explode($delimiter, $object->url);
            if (!empty($explURL)) {
                if (!empty($explURL[1])) {
                    $object->type = $validType;
                } else {
                    $object->type = self::TYPE_UNKNOWN;
                }
            } else {
                $object->type = self::TYPE_UNKNOWN;
            }

            /* Return objecf */
            return $object;
        }

        /* Set some data */
        $object->scheme = parse_url($object->url, PHP_URL_SCHEME);
        $object->host = parse_url($object->url, PHP_URL_HOST);
        $object->port = parse_url($object->url, PHP_URL_PORT);

        /* Check if host is valid domain name */
        $domain_validation = '/((http|https)\:\/\/)?[a-zA-Z0-9\.\/\?\:@\-_=#]+\.([a-zA-Z0-9\&\.\/\?\:@\-_=#])*/';
        if (preg_match("$domain_validation", $object->host)) {
            /* Host is domain */
            $object->type = self::TYPE_DOMAIN;

            /* Get doamin */
            $explHost = explode('.', $object->host);
            $explCount = count($explHost);
            if ($explCount > 2) {
                $hostname = $explHost[($explCount - 2)];
                $extension = $explHost[($explCount - 1)];
                $object->domain = $hostname . '.' . $extension;
            } else {
                $object->domain = $object->host;
            }
        } else {
            /* Hostname is IP */
            $object->domain = $object->host;
            $object->type = self::TYPE_OTHER;
        }

        /* Set path */
        $path = parse_url($url, PHP_URL_PATH);
        $object->path = ($path === '/') ? '' : $path;

        /* Set simple URL */
        $object->simpleURL = $object->scheme . '://' . $object->host . '/';

        /* Return object */
        return $object;
    }

    public function retrieve() : array {
        /* Init CURL */
        $ch = curl_init($this->_url);
        curl_setopt($ch, CURLOPT_USERAGENT, "Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 6.0)");
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, TRUE);
        curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, FALSE);
        curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, FALSE);
        curl_setopt($ch, CURLOPT_MAXREDIRS, 10);
        curl_setopt($ch, CURLOPT_FOLLOWLOCATION, TRUE);
        curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, 60);

        /* Get HTML page */
        $this->_content = curl_exec($ch);
        if (curl_error($ch)) {
            return [FALSE, curl_error($ch)];
        }

        /* Get the status code */
        $this->_httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);

        /* Get status */
        $status = FALSE;
        $message = '';
        switch ($this->_httpCode) {
            case 200:
            case 301:
                $status = TRUE;
                break;
            default:
                $message = 'Unsupported HTTP code: ' . $this->_httpCode;
                break;
        }

        /* Close CURL */
        curl_close($ch);

        /* Return status */
        return [$status, $message];
    }

    public function getId() {
        return $this->_id;
    }

    public function getHost() {
        return $this->_host;
    }

    public function getSimpleURL() {
        return $this->_simpleURL;
    }

    public function convertToSmartDOM() : array {
        /* Check if content is empty */
        if ($this->_content === NULL) {
            return [NULL, 'Content is empty'];
        }

        /* Return smart DOM object */
        $smartDOM = new SmartDOM;
        list($status, $message) = $smartDOM->load($this->_content);
        if (!$status) {
            return [NULL, $message];
        }

        /* Success */
        return [$smartDOM, ''];
    }
}
