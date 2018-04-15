<?php

namespace OpenHappen\Bot;

class SmartDOM {

    protected $_domDocument = NULL;

    public function __construct() {
        $this->_domDocument = new \DOMDocument;
    }

    public function load(string $content) {
        $status = @$this->_domDocument->loadHTML($content);
        if (!$status) {
            return [FALSE, 'Failed to load HTML'];
        }

        /* Success */
        return [TRUE, ''];
    }

    public function getHrefs($filters = []) {
        /* Check if filter is active */
        $activeFilters = !empty($filters);

        /* Get hrefs */
        $hrefs = [];
        $elements = $this->_domDocument->getElementsByTagName('a');
        foreach ($elements as $element) {
            $href = $element->getAttribute('href');
            if (!empty($href)) {
                /* Create Href  */
                $href = new Href($href);

                /* Get type and check if type is valid or exists in filter */
                $type = $href->getType();
                if ($type !== Request::TYPE_UNKNOWN &&
                    (!$activeFilters || ($activeFilters && in_array($type, $filters)))) {
                    $hrefs[] = $href;
                }
            }
        }

        /* Return hrefs */
        return $hrefs;
    }
}
