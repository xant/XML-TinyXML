package XML::TinyXML::Selector::XPath::Axis;

sub child {
    my ($class, $context) = @_;
    return $context->{node}->children;
}

sub descendant {
    my ($class, $context) = @_;
    my @res = $context->{node}->children;
    foreach my $child (@res) {
        push (@res, descendant({ node => $child }));
    }
    return wantarray?@res:\$res;
}

sub parent {
    my ($class, $context) = @_;
    return $context->parent;
}

sub ancestor {
    my ($class, $context) = @_;
    my $node = $context->{node};
    my @res;
    while ($node) {
        my $parent = $node->parent;
        push (@res, $parent) if ($parent);
        $node = $parent;
    }
    return wantarray?@res:\@res;
}

sub following_sibling {
    my ($class, $context) = @_;
    my $node = $context->{node};
    my @res;
    while ($node) {
        my $next = $node->nextSibling;
        push (@res, $next) if $next;
        $node = $next;
    }
    return wantarray?@res:\@res;
}

sub preceding_sibling {
    my ($class, $context) = @_;
    my $node = $context->{node};
    my @res;
    while ($node) {
        my $prev = $node->prevSibling;
        push (@res, $prev) if $prev;
        $node = $prev;
    }
    return wantarray?@res:\@res;
}

sub attribute {
    my ($class, $context) = @_;
    return $context->{node}->attributes;
}

sub self {
    my ($class, $context) = @_;
    return $context->{node};
}

sub descendant_or_self {
    my ($class, $context) = @_;
    my @res = descendant($context);
    unshift(@res, $context->{node});
    return wantarray?@res:\@res;
}

sub ancestor_or_self {
    my ($class, $context) = @_;
    my @res = ancestor($context);
    unshift(@res, $context->{node});
    return wantarray?@res:\@res;
}


1;
