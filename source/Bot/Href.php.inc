<?php

declare(strict_types=1);

namespace OpenHappen\Bot;

class Href {

    const CHANGE_FREY_ALWAYS = 'always';
    const CHANGE_FREY_HOURLY = 'hourly';
    const CHANGE_FREY_DAILY = 'daily';
    const CHANGE_FREY_WEEKLY = 'weekly';
    const CHANGE_FREY_MONTHLY = 'monthly';
    const CHANGE_FREY_YEARLY = 'yearly';
    const CHANGE_FREY_NEVER = 'never';

    protected $_timezone = NULL;

    protected $_url = NULL;
    protected $_host = NULL;
    protected $_type = FALSE;

    protected $_lastModified = NULL;
    protected $_changeFrey = 'monthly';

    public function __construct(string $url, $lastModified = NULL, $changeFrey = self::CHANGE_FREY_MONTHLY) {
        /* Set request info */
        $object = Request::ParseURL($url);
        $this->_url = $object->url;
        $this->_host = $object->host;
        $this->_type = $object->type;

        /* Set timezone */
        $this->_timezone = new \DateTimeZone('UTC');

        /* Check if lastModified is not empty */
        if ($lastModified !== NULL) {
            try {
                $this->_lastModified = new \DateTime($lastModified, $this->_timezone);
            } catch (\Exception $e) {
                $this->_lastModified = new \DateTime('now', $this->_timezone);
            }
        } else {
            $this->_lastModified = new \DateTime('now', $this->_timezone);
        }

        /* Set change frey */
        if ($changeFrey !== NULL && in_array($changeFrey, $this->getChangeFreqTypes())) {
            $this->_changeFrey = $changeFrey;
        }
    }

    public function getChangeFreqTypes() {
        return [
            self::CHANGE_FREY_ALWAYS,
            self::CHANGE_FREY_HOURLY,
            self::CHANGE_FREY_DAILY,
            self::CHANGE_FREY_WEEKLY,
            self::CHANGE_FREY_MONTHLY,
            self::CHANGE_FREY_YEARLY,
            self::CHANGE_FREY_NEVER
        ];
    }

    public function getType() : string {
        return $this->_type;
    }

    public function getChangeFreq() {
        return $this->_changeFrey;
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

    public function getNextExport() {
        /* Create date object */
        $dateTime = new \DateTime('now', $this->_timezone);
        $dateTime->setTimestamp($this->_lastModified->getTimestamp());

        /* Get current time stamp */
        switch ($this->_changeFrey) {
            case self::CHANGE_FREY_HOURLY:
                $dateTime->modify('+1 hour');
                break;
            case self::CHANGE_FREY_DAILY:
                $dateTime->modify('+1 day');
                break;
            case self::CHANGE_FREY_WEEKLY:
                $dateTime->modify('+1 week');
                break;
            case self::CHANGE_FREY_MONTHLY:
                $dateTime->modify('+1 month');
                break;
            case self::CHANGE_FREY_YEARLY:
                $dateTime->modify('+1 year');
                break;
            default:
                return 0;
        }

        /* Get next export time */
        return $dateTime->getTimestamp();
    }

    public function toStdClass(string $domainURL) : \stdClass {
        /* Create object */
        $object = new \stdClass;
        $object->url = $this->getURL($domainURL);

        /* Return object */
        return $object;
    }
}
