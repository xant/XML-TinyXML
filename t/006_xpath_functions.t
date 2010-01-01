
use strict;
use Test::More tests => 20;
use XML::TinyXML;
use XML::TinyXML::Selector;
BEGIN { use_ok('XML::TinyXML::Selector::XPath::Functions') };

my $txml = XML::TinyXML->new();
my $selector = XML::TinyXML::Selector->new($txml, "XPath");

# contains()
is (XML::TinyXML::Selector::XPath::Functions->contains($selector->context, "TEST", "ES"), 1);
is (XML::TinyXML::Selector::XPath::Functions->contains($selector->context, "TEST", "es"), 0);

# starts-with()
is (XML::TinyXML::Selector::XPath::Functions->starts_with($selector->context, "TEST", "TE"), 1);
is (XML::TinyXML::Selector::XPath::Functions->starts_with($selector->context, "TEST", "ST"), 0);

# translate()
is (XML::TinyXML::Selector::XPath::Functions->translate($selector->context, "bar", "abc", "ABC"), "BAr");
is (XML::TinyXML::Selector::XPath::Functions->translate($selector->context, "--aaa--","abc-","ABC"), "AAA");

# substring-before()
is (XML::TinyXML::Selector::XPath::Functions->substring_before($selector->context, "1999/04/01", "/"), "1999");

# substring-after()
is (XML::TinyXML::Selector::XPath::Functions->substring_after($selector->context, "1999/04/01", "/"), "04/01");
is (XML::TinyXML::Selector::XPath::Functions->substring_after($selector->context, "1999/04/01", "19"), "99/04/01");

# substring()
is (XML::TinyXML::Selector::XPath::Functions->substring($selector->context, "12345", 2), "2345");
is (XML::TinyXML::Selector::XPath::Functions->substring($selector->context, "12345", 0, 3), "12");
is (XML::TinyXML::Selector::XPath::Functions->substring($selector->context, "12345",2,3), "234");
is (XML::TinyXML::Selector::XPath::Functions->substring($selector->context, "12345", 1.5, 2.6), "234");
is (XML::TinyXML::Selector::XPath::Functions->substring($selector->context, "12345", "5 div 5", 2), "12");
is (XML::TinyXML::Selector::XPath::Functions->substring($selector->context, "12345", "0 div 3", 2), "12");
is (XML::TinyXML::Selector::XPath::Functions->substring($selector->context, "12345", "0 div 0", 3), "");
is (XML::TinyXML::Selector::XPath::Functions->substring($selector->context, "12345", 1, "0 div 0"), "");
is (XML::TinyXML::Selector::XPath::Functions->substring($selector->context, "12345", -42, "0 div 0"), "12345");
is (XML::TinyXML::Selector::XPath::Functions->substring($selector->context, "12345", "-1 div 0", "1 div 0"), "");

