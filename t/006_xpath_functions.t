
use strict;
use Test::More tests => 7;
use XML::TinyXML;
BEGIN { use_ok('XML::TinyXML::Selector::XPath::Functions') };



is (XML::TinyXML::Selector::XPath::Functions->translate(undef, "bar", "abc", "ABC"), "BAr");
is (XML::TinyXML::Selector::XPath::Functions->translate(undef, "--aaa--","abc-","ABC"), "AAA");

is (XML::TinyXML::Selector::XPath::Functions->substring(undef, "12345",2), "2345");
is (XML::TinyXML::Selector::XPath::Functions->substring(undef, "12345", 0, 3), "12");
is (XML::TinyXML::Selector::XPath::Functions->substring(undef, "12345",2,3), "234");
is (XML::TinyXML::Selector::XPath::Functions->substring(undef, "12345", 1.5, 2.6), "234");

