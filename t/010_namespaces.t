
use strict;
use Test::More tests => 2;
use XML::TinyXML;
use XML::TinyXML::Selector;

my $txml = XML::TinyXML->new();
$txml->loadFile("./t/t.xml");

my $node = $txml->getNode("/xml/parent[2]");
is ($node->nameSpace->uri, "foo://bar");
$node = $txml->getNode("/xml/parent[1]");
is ($node->nameSpace->uri, "bar://foo");

