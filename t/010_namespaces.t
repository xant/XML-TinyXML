
use strict;
use Test::More tests => 16;
use XML::TinyXML;
use XML::TinyXML::Selector;
use Data::Dumper;

my $txml = XML::TinyXML->new();
$txml->loadFile("./t/t.xml");

my $node = $txml->getNode("/xml/parent[2]");
is ($node->namespace->uri, "foo://bar");
$node = $txml->getNode("/xml/parent[1]");
is ($node->namespace->uri, "bar://foo");
my $child = $node->getChildNodeByName("child1");
is ($child->namespace->uri, "bar://child");

my $txml2 = XML::TinyXML->new();
$txml2->loadFile("./t/t2.xml");
my $node2 = $txml2->getNode("/xml");
is ($node->namespace->name, "bar");
$node2->addChildNode($node);
is ($node->namespace->name, "bar"); # moving a node across different contextes
$child = $txml2->getNode("/xml/parent/child1");
is ($child->namespace->uri, "bar://child");

$node = $txml->getNode("/xml/hello");
is ($node->namespace->uri, "foo://bar");
# the node will hinerit the default namespace of the new document
$node2->addChildNode($node); 
is ($node->namespace->uri, "bar://foo");
$child = $txml2->getNode("/xml/parent/child2");
is ($child->namespace->name, "special_child");

#my @namespaces = $txml2->getNode("/xml/hello")->knownNamespaces;
#printf("%s -> %s\n", $_->name, $_->uri) for @namespaces;
#warn $txml2->getNode("/xml/hello")->hineritedNamespace->uri;

$txml->loadFile("./t/ns.xml");
$node = $txml->getNode("/root/g/p");
is ($node->hineritedNamespace, undef);
is ($node->namespace->uri, "http://www.example.org/a");
$node = $txml->getNode("/root/g1/p");
is ($node->namespace->uri, "http://www.example.org/x");
is ($node->hineritedNamespace->uri, $node->namespace->uri);
$node = $txml->getNode("/root/g3/div");
is ($node->namespace->uri, "http://www.example.org/a");
# check if resetting the default namespace works
$node = $txml->getNode("/root/g3/reset");
is ($node->namespace, undef);
$node = $txml->getNode("/root/g4/p");
ok ($node->namespace->uri eq $node->hineritedNamespace->uri);



#warn $txml->dump;
#printf("%s -> %s\n", $_->name, $_->uri) for $node->knownNamespaces;

