
use strict;
use Test::More tests => 8;
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

my $txml2 = XML::TinyXML->new();
$txml2->loadFile("./t/t2.xml");
my $node2 = $txml2->getNode("/xml");
is ($node->nameSpace->name, "bar");
$node2->addChildNode($node);
is ($node->nameSpace->name, "bar2");
$child = $txml2->getNode("/xml/parent/child1");
is ($child->nameSpace->uri, "bar://child");

$node = $txml->getNode("/xml/hello");
is ($node->nameSpace->uri, "foo://bar");
# the node will hinerit the default namespace of the new document
$node2->addChildNode($node); 
is ($node->nameSpace->uri, "bar://foo");
