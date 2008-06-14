# -*- tab-width: 4 -*-
# ex: set tabstop=4:

=head1 NAME

XML::TinyXML::Node - Tinyxml Node object

=head1 SYNOPSIS

=over 4

  use XML::TinyXML;

  # first obtain an xml context:
  $xml = XML::TinyXML->new("rootnode", "somevalue", { attr1 => v1, attr2 => v2 });

  # We create a node and later attach it to a parent one doing:
  $child_node = XML::TinyXML::Node->new("child", "somevalue");
  ... [ some code ] ...
  $parent_node->addChildNode($child_node);

  # or if we want to create a childnode in a single statemnet
  $child_node = XML::TinyXML::Node->new("child", "somevalue", $parent_node);

  # we can later retrive the "child" node by calling
  $child_node = $xml->getNode("/nodelabel/child");

  # at this point , calling :
  print $xml->dump;
  # would produce the following xml
  #
  # <?xml version="1.0"?>
  # <rootnode>
  #   <child>othervalue</child>
  # </rootnode>
  
=back

=head1 DESCRIPTION

Node representation for the TinyXML API

=head1 INSTANCE VARIABLES

=over 4

=item * _node

Reference to the underlying XmlNodePtr object (which is a binding to the XmlNode C structure)

=back

=head1 METHODS

=over 4

=cut
package XML::TinyXML::Node;

use strict;
our $VERSION = '0.04';

=item * new ($entity, $value, $parent, %attrs)

Creates a new XML::TinyXML::Node object.

$entity can be either a scalar or an XmlNodePtr object.
    - if it's a scalar , it will be intepreted as the entity name 
    - if it's an XmlNodePtr, it will be used as the underlying object
      and will be incapsulated in the newly created XML::TinyXML::Node object.

$value is the optianal string value of the newly created node (the "content" of
the xml node)

if $parent isn't undef the newly created node will be directly attached to 
the specified parent node. $parent can be either a XML::TinyXML::Node object
or a XmlNodePtr one.

%attrs is an optional hash specifying attributes for the newly created xml node

Returns a valid XML::TinyXML::Node object

=cut
sub new {
    my ($class, $entity, $value, $parent, %attrs) = @_;
    return undef unless($entity);
    my $node = undef;
    if(ref($entity) && UNIVERSAL::isa($entity, "XmlNodePtr")) {
        $node = $entity;
    } else {
        $node = XML::TinyXML::XmlCreateNode($entity, $value || "");
    }
    return undef unless($node);
    if(ref($parent)) {
        my $pnode = undef;
        if(UNIVERSAL::isa($parent, "XmlNodePtr")) {
            $pnode = $parent;
        } elsif(UNIVERSAL::isa($parent, "XML::TinyXML::Node")) {
            $pnode = $parent->{_node};
        }
        if($pnode) {
            XML::TinyXML::XmlAddChildNode($pnode, $node);
        } else {
        }
    } 
    my $self = {};
    bless($self, $class);
    $self->{_node} = $node;
    if(%attrs) {
        $self->AddAttributes(%attrs);
    }
    $self;
}

=item * cleanAttributes ()

Removes all node attributes

=cut
sub cleanAttributes {
    my ($self) = @_;
    return XML::TinyXML::XmlClearAttributes($self->{_node});
}

=item * removeAttribute ($index)

Removes attribute at $index

=cut
sub removeAttribute {
    my ($self, $index) = @_;
    return XML::TinyXML::XmlRemoveAttribute($self->{_node}, $index);
}

#=item * removeAttributeByName ($name)
#=cut

=item * loadHash ($hashref, [ $childname ])

Loads an hashref and represent it as an xml subbranch.

$hashref 

if $childname is specified, newly created childnodes will use it as their name

=cut
sub loadHash {
    my ($self, $hash, $childname) = @_;
    foreach my $k (keys(%$hash)) {
        my $name = $childname || $k;
        if(!ref($hash->{$k}) || ref($hash->{$k}) eq "SCALAR") {
            $self->addChildNode(XML::TinyXML::Node->new($name, $hash->{$k}));
        } elsif(ref($hash->{$k}) eq "HASH") {
            my $child = XML::TinyXML::Node->new($name);
            $self->addChildNode($child);
            $child->loadHash($hash->{$k});
        } elsif(ref($hash->{$k}) eq "ARRAY") {
            foreach my $entry (@{$hash->{$k}}) {
                #warn "Anonymous/Nested arrayrefs are flattened !!! This should be fixed in the future";
                #$self->parent->addChildNode($childname);
                $self->loadHash({ __import__ => $entry }, $name);
            }
        }
    }
}

=item * toHash ([ $parent ])

Export the xml structure into an hashref (formerly the inverse of loadHash)

if $parent is specified the resulting structure will be connected to $parent.
(NOTE: $parent MUST obviously be an hashref)

=cut
sub toHash {
    my ($self, $parent) = @_;
    my $hashref = {};
    foreach my $child ($self->children) {
        my $key = $child->name;
        my $value = $child->value;
        if($child->countChildren) { 
            $value = $child->toHash($hashref);
        }
        if($hashref->{$key}) {
            if(ref($hashref->{$key}) eq "ARRAY") {
                push(@{$hashref->{$key}}, $value);
            } else {
                $hashref->{$key} = [ $hashref->{$key}, $value ];
            }
        } else {
            $hashref->{$key} = $value;
        }
    }
    if($parent && $self->value) {
        if($parent->{$self->{name}}) {
            if(ref($parent->{$self->name} eq "ARRAY")) {
                push(@{$parent->{$self->name}}, $self->value);
            } else {
                $parent->{$self->name} = [ $parent->{$self->name}, $self->value ];
            }
        } else {
            $parent->{$self->name} = $self->value;
        }
    }
    return $hashref;
}

=item * updateAttributes (%attrs)

Updates all attributes.

This method simply clean all current attributes and replace them with 
the ones specified in the %attrs hash

=cut
sub updateAttributes {
    my ($self, %attrs) = @_;
    XML::TinyXML::XmlClearAttributes($self->{_node});
    $self->addAttributes(%attrs);
}

=item * addAttributes (%attrs)

Add attributes.

=cut
sub addAttributes {
    my ($self, %attrs) = @_;
    foreach my $key (keys %attrs) {
        XML::TinyXML::XmlAddAttribute($self->{_node}, $key, $attrs{$key});
    }
}

=item * name ([$newname])

Set/Get the name of a node.
if $newname is specified it will be used as the new name, 
otherwise the current name is returned

=cut
sub name { 
    my ($self, $newname) = @_;
    $self->{_node}->name($newname) 
        if($newname);
    return $self->{_node}->name;
}

=item * value ([$newval])

Set/Get the vlue of a node.
if $newval is specified it will be used as the new value, 
otherwise the current value is returned

=cut
sub value {
    my ($self, $newval) = @_;
    $self->{_node}->value($newval) 
        if($newval);
    return $self->{_node}->value;
}

=item * attributes ()

Read-Only method to obtain an hashref to the attributes of this node

=cut
sub attributes {
    my ($self) = shift;
    my $res = {};
    for(my $i = 1; $i <= XML::TinyXML::XmlCountAttributes($self->{_node}); $i++) {
        my $attr = XML::TinyXML::XmlGetAttribute($self->{_node}, $i);
        $res->{$attr->name} = $attr->value;
    }
    return $res;
}

=item * getChildNode ($index)

Returns child node at $index.
The returned node will be a Xml::TinyXML::Node object

=cut
sub getChildNode {
    my ($self, $index) = @_;
    return XML::TinyXML::Node->new(XML::TinyXML::XmlGetChildNode($self->{_node}, $index));
}

=item * getChildNodeByName ($name)

Returns the first child node whose name matches $name.
The returned node will be a Xml::TinyXML::Node object

=cut
sub getChildNodeByName {
    my ($self, $name) = @_;
    return undef unless($name);
    return XML::TinyXML::Node->new(XML::TinyXML::XmlGetChildNodeByName($self->{_node}, $name));
}

=item * countChildren ()

Returns the actual number of children

=cut
sub countChildren {
    my $self = shift;
    return XML::TinyXML::XmlCountChildren($self->{_node});
}

=item * children ()

Returns an array containing all actual children in the form of Xml::TinyXML::Node objects

=cut
sub children {
    my ($self) = @_;
    my @children = ();
    for(my $i = 1; $i <= XML::TinyXML::XmlCountChildren($self->{_node}); $i++) {
        push(@children, XML::TinyXML::Node->new(XML::TinyXML::XmlGetChildNode($self->{_node}, $i)));
    }
    return @children;
}

=item * addChildNode ($child)

Adds a new child node.

$child MUST be a XML::TinyXML::Node object

=cut
sub addChildNode {
    my ($self, $child) = @_;
    return undef unless($child && UNIVERSAL::isa($child, "XML::TinyXML::Node"));
    return XML::TinyXML::XmlAddChildNode($self->{_node}, $child->{_node});
}

=item * parent ()

Read-Only method which returns the parent node in the form of a XML::TinyXML::Node object.

=cut
sub parent {
    my ($self) = @_;
    return XML::TinyXML::Node->new($self->{_node}->parent);
}

=item * type ()

Returns the "type" of a XML::TinyXML::Node object.
type can be :
    NODE
    COMMENT
    CDATA

=cut
sub type {
    my ($self) = @_;
    my $type = $self->{_node}->type;
    if($type == XML::TinyXML::XML_NODETYPE_SIMPLE()) {
        $type = "NODE";
    } elsif ($type == XML::TinyXML::XML_NODETYPE_COMMENT()) {
        $type = "COMMENT";
    } elsif ($type == XML::TinyXML::XML_NODETYPE_CDATA()) {
        $type = "CDATA";
    }
    return $type;
}

1;

=back

=head1 SEE ALSO

=over 4

XML::TinyXML

=back

=head1 AUTHOR

xant, E<lt>xant@cpan.orgE<gt>

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2008 by xant

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself, either Perl version 5.8.8 or,
at your option, any later version of Perl 5 you may have available.


=cut
