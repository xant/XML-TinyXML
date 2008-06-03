# -*- tab-width: 4 -*-
# ex: set tabstop=4:

=head1 NAME

TinyXML::Node

=head1 VERSION

I<$Id$>

=head1 SYNOPSIS

=over 4

  use TinyXML;

  # first obtain an xml context:
  $xml = TinyXML->new("rootnode", "somevalue", { attr1 => v1, attr2 => v2 });

  # if we create a childnode
  $child = TinyXML::Node->new("child", "somevalue");
  $node->addChildNode($child);
  # we can later retrive the "child" node calling
  $child = $xml->getNode("/nodelabel/child");

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

Tera main object

=head1 INSTANCE VARIABLES

=over 4

=item * _node

Reference to the underlying XmlNodePtr object (which is a binding to the XmlNode C structure)

=back

=head1 METHODS

=over 4

=cut
package TinyXML::Node;

use strict;
our $VERSION = $TinyXML::Version;

=item * new ($name, $value, %attrs)

=cut
sub new {
    my ($class, $name, $value, %attrs) = @_;
    return undef unless($name);
    my $node = undef;
    if(ref($name) && UNIVERSAL::isa($name, "XmlNodePtr")) {
        $node = $name;
    } else {
        $node = TinyXML::XmlCreateNode($name, $value || "");
    }
    return undef unless($node);
    my $self = {};
    bless($self, $class);
    $self->{_node} = $node;
    if(%attrs) {
        $self->AddAttributes(%attrs);
    }
    $self;
}

=item * cleanAttributes

=cut
sub cleanAttributes {
}

sub removeAttribute {
    my ($self, $index) = @_;
    return TinyXML::XmlRemoveAttribute($self->{_node}, $index);
}

=item * loadHash ($hash, [ $nest ])

=cut
sub loadHash {
    my ($self, $hash, $nest) = @_;
    foreach my $k (keys(%$hash)) {
        my $name = $nest || $k;
        if(!ref($hash->{$k}) || ref($hash->{$k}) eq "SCALAR") {
            $self->addChildNode(TinyXML::Node->new($name, $hash->{$k}));
        } elsif(ref($hash->{$k}) eq "HASH") {
            my $child = TinyXML::Node->new($name);
            $self->addChildNode($child);
            $child->loadHash($hash->{$k});
        } elsif(ref($hash->{$k}) eq "ARRAY") {
            foreach my $entry (@{$hash->{$k}}) {
                #warn "Anonymous/Nested arrayrefs are flattened !!! This should be fixed in the future";
                #$self->parent->addChildNode($nest);
                $self->loadHash({ __import__ => $entry }, $name);
            }
        }
    }
}

=item * toHash ([ $parent ])

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

=cut
sub updateAttributes {
    my ($self, %attrs) = @_;
    TinyXML::XmlClearAttributes($self->{_node});
    $self->addAttributes(%attrs);
}

=item * addAttributes (%attrs)

=cut
sub addAttributes {
    my ($self, %attrs) = @_;
    foreach my $key (keys %attrs) {
        TinyXML::XmlAddAttribute($self->{_node}, $key, $attrs{$key});
    }
}

=item * name ([$newname])

=cut
sub name { 
    my ($self, $newname) = @_;
    $self->{_node}->name($newname) 
        if($newname);
    return $self->{_node}->name;
}

=item * value ([$newval])

=cut
sub value {
    my ($self, $newval) = @_;
    $self->{_node}->value($newval) 
        if($newval);
    return $self->{_node}->value;
}

=item * attributes ()

=cut
sub attributes {
    my ($self) = shift;
    my $res = {};
    for(my $i = 1; $i <= TinyXML::XmlCountAttributes($self->{_node}); $i++) {
        my $attr = TinyXML::XmlGetAttribute($self->{_node}, $i);
        $res->{$attr->name} = $attr->value;
    }
    return $res;
}

sub getChildNode {
    my ($self, $index) = @_;
    return TinyXML::Node->new(TinyXML::XmlGetChildNode($self->{_node}, $index));
}

sub countChildren {
    my $self = shift;
    return TinyXML::XmlCountChildren($self->{_node});
}

sub children {
    my ($self) = @_;
    my @children = ();
    for(my $i = 1; $i <= TinyXML::XmlCountChildren($self->{_node}); $i++) {
        push(@children, TinyXML::Node->new(TinyXML::XmlGetChildNode($self->{_node}, $i)));
    }
    return @children;
}

=item * addChildNode ($child)

=cut
sub addChildNode {
    my ($self, $child) = @_;
    return undef unless($child && UNIVERSAL::isa($child, "TinyXML::Node"));
    return TinyXML::XmlAddChildNode($self->{_node}, $child->{_node});
}

=item * parent ($child)

=cut
sub parent {
    my ($self) = @_;
    return TinyXML::Node->new($self->{_node}->parent);
}

=item * type ()
Returns the "type" of a TinyXML::Node object.
type can be :
    NODE
    COMMENT
    CDATA
=cut
sub type {
    my ($self) = @_;
    my $type = $self->{_node}->type;
    if($type == TinyXML::XML_NODETYPE_SIMPLE()) {
        $type = "NODE";
    } elsif ($type == TinyXML::XML_NODETYPE_COMMENT()) {
        $type = "COMMENT";
    } elsif ($type == TinyXML::XML_NODETYPE_CDATA()) {
        $type = "CDATA";
    }
    return $type;
}

1;

=back

=head1 SEE ALSO

=over 4

TinyXML

=back

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2008 by xant

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself, either Perl version 5.8.8 or,
at your option, any later version of Perl 5 you may have available.


=cut
