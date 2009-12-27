use Test::More tests => 3;
BEGIN { use_ok('XML::TinyXML') };

$txml = XML::TinyXML->new();
$txml->loadBuffer("<node>Import&amp;special&quot;&lt;chars&gt;&#67;&#105;&#97;&#111;</node>");
$node = $txml->getRootNode(1);
ok ( $node->value eq "Import&special\"<chars>Ciao", "unescaping" );

$txml = XML::TinyXML->new();
$txml->addRootNode("nodelabel", "some'&'value", { attr1 => 'v>1', attr2 => 'v<2' });
#print $txml->dump;
ok ( $txml->dump eq 
q~<?xml version="1.0"?>
<nodelabel attr1="v&gt;1" attr2="v&lt;2">some&apos;&amp;&apos;value</nodelabel>
~, 'escaping');


