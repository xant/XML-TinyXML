
use strict;
use Test::More tests => 3;
use XML::TinyXML;
use XML::TinyXML::Selector;

my $txml = XML::TinyXML->new();
$txml->loadFile("./t/t.xml");

my $node = $txml->getNode("/xml/parent[2]");
is ($node->nameSpace->uri, "foo://bar");
$node = $txml->getNode("/xml/parent[1]");
is ($node->nameSpace->uri, "bar://foo");
my $child = $node->getChildNodeByName("child1");
is ($child->nameSpace->uri, "bar://child");
