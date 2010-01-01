package XML::TinyXML::Selector::XPath::Functions;

use POSIX qw(ceil floor);

# NODE FUNCTIONS

sub last {
    my ($class, $context) = @_;
    return scalar(@{$context->{items}}) -1
        if ($context and $context->{items} and ref($context->{items}) eq "ARRAY");
}

sub position {
    my ($class, $context) = @_;
    my $cnt = 0;
    return map { ++$cnt => $_ } @{$context->{items}};
}

sub count {
    my ($class, $context, $items) = @_;
    return scalar(@{$items});
}

sub id {
    my ($class, $context, $id, $cnode) = @_;
    foreach my $child ($cnode?$cnode->children:$context->{xml}->rootNodes) {
        my @selection;
        if ($child->attributes->{id} and $child->attributes->{id} eq $id) {
            return $child;
        }
        return &id($context, $child);
    }
}

sub local_name {
    my ($class, $context, $items) = @_;
    return map { $_->name } $items?@$items:($context->{node});
}

sub name {
    # XXX - out of spe
    return local_name(@_);
}

# STRING FUNCTIONS

sub string {
    my ($class, $context, $items) = @_;
    return map { $_->value } $items?@$items:($context->{node});
}

sub concat {
    my ($class, $context, $str1, $str2) = @_;
    return $str1.$str2;
}

sub starts_with {
    my ($class, $context, $str1, $str2) = @_;
    return ($str1 =~ /^$str2/);
}

sub contains {
    my ($class, $context, $str1, $str2) = @_;
    return ($str1 =~ /$str2/);
}

sub substring_before {
    my ($class, $context, $str1, $str2) = @_;
    my ($match) = $str1 =~ /(.*?)$str2/;
    return $match;
}

sub substring_after {
    my ($class, $context, $str1, $str2) = @_;
    my ($match) = $str1 =~ /$str2(.*)/;
    return $match;
}

sub substring {
    my ($class, $context, $str, $offset, $length) = @_;
    return defined($length)
            ? substr($str, $offset, $length)
            : substr($str, $offset);
}

sub string_length {
    my ($class, $context, $str) = @_;
    return length($str);
}

sub normalize_space {
    my ($class, $context, $str) = @_;
    $str =~ s/(^\s+|\s+$)//g;
    return $str;
}

sub translate {
    my ($class, $context, $str, $tfrom, $tto) = @_;

    my @from = split(//, $tfrom);
    my @to = split(//, $tto);
    foreach my $i (0..$#from) {
        $str =~ s/$from[$i]/$to[$i]/g;
    }
    return $str;
}

# BOOLEAN FUNCTIONS

sub boolean {
    my ($class, $context, $item) = @_;
    return $item?1:0;
}

sub not {
    my ($class, $context, $item) = @_;
    return !$item?1:0;
}

sub true {
    return 1;
}

sub falce {
    return 0;
}

sub lang {
    my ($class, $context, $lang) = @_;
    # TODO - implement;
    warn __PACKAGE__."::lang() unimplemented";
}

# NUMBER FUNCTIONS

sub number {
    my ($class, $context, $item) = @_;
    return 0+$item; # force numeric context
}

sub sum {
    my ($class, $context, $items) = @_;
    my $res = 0;
    if ($items) {
        $res += $_->value for (@$items);
    }
    return $res;
}

sub floor {
    my ($class, $context, $number) = @_;
    return floor($number);
}

sub ceil {
    my ($class, $context, $number) = @_;
    return ceil($number);
}

sub round {
    my ($class, $context, $number) = @_;
    return int($number + .5 * ($number <=> 0));
}

1;
