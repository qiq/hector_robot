# DetectLanguageTable.pm, simple, perl
# Detect language in doc/paragraphs using top most common words for every language.
# 
# Dependencies:
# 
# Parameters:
# items			r/o	n/a	Total items processed
# filenamePrefix	r/o	n/a	Files containing most common words, one
#					word per line.
# defaultLanguage	r/w	n/a	Default language (if not sure or equal score).
# paragraphLevel	r/w	1	Work on paragraph level (otherwise whole document)
# 
# Status:
# not changed

package DetectLanguageTable;

use warnings;
use strict;
use HectorRobot;
use Module;
our @ISA = qw(Module);
use HectorRobotCommon;

binmode STDOUT, ":utf8";

sub new {
	my ($proto, $object, $id, $threadIndex) = @_;
	my $class = ref($proto) || $proto;
	my $self = $class->SUPER::new('SIMPLE', $object, $id, $threadIndex);

	$self->{'items'} = 0;
	$self->{'filenamePrefix'} = '';
	$self->{'defaultLanguage'} = '';
	$self->{'paragraphLevel'} = 1;

	$self->{'_words'} = {};

	bless($self, $class);
	return $self;
}

sub Init {
	my ($self, $params) = @_;

	# second stage?
	return 1 if (not defined $params);

	foreach my $p (@{$params}) {
		if (exists $self->{$p->[0]}) {
			$self->{$p->[0]} = $p->[1];
		}
	}

	if ($self->{'filenamePrefix'} eq "") {
		$self->LOG_ERROR("filenamePrefix not defined");
		return 0;
	}
	foreach my $f (glob($self->{'filenamePrefix'}.'.*')) {
		$f =~ /.*\.([^\.]*)$/;
		my $lang = $1;
		my $fd;
		if (not open($fd, "<:encoding(UTF-8)", $f)) {
			$self->LOG_ERROR("Cannot open file: $f: $!");
			return 0;
		}
		while (<$fd>) {
			chomp;
			#/(.*)\t(.*)/ or die;
			#push(@{$self->{'_words'}->{$1}}, [ $lang, $2 ]);
			push(@{$self->{'_words'}->{$1}}, $lang);
		}
		close($fd);
	}

	return 1;
}

#binmode STDERR, ":utf8";
sub ProcessSimple() {
	my ($self, $resource) = @_;

	return $resource if ($resource->GetTypeString() ne 'TextResource');
	my $tr = HectorRobot::ResourceToTextResource($resource);

	my $nWords = $tr->GetFormCount();
	my @lang;
	my %score;
	my $words =  0;
	for (my $i = 0; $i < $nWords; $i++) {
		my $flags = $tr->GetFlags($i);
		my $word = $tr->GetForm($i);
		utf8::decode($word);
		if ($flags & $HectorRobot::TextResource::TOKEN_PARAGRAPH_START and $i > 0 and $self->{'paragraphLevel'}) {
#use Data::Dumper;
#print Dumper(\%score);
			my $lang = $self->{'defaultLanguage'} ne '' ? $self->{'defaultLanguage'} : '?';
			my $max = exists $score{$lang} ? $score{$lang} : -1;
			foreach my $k (keys %score) {
#print STDERR "$k, ".$score{$k}."\n";
				if ($score{$k} > $max) {
					$max = $score{$k};
					$lang = $k;
				}
			}
#print STDERR "MAX: $lang, $max\n";
			push(@lang, $lang);
			%score = ();
		}
#print STDERR "$word\n" if (not exists $self->{'_words'}->{lc($word)});
		if (exists $self->{'_words'}->{lc($word)}) {
			my @langs = @{$self->{'_words'}->{lc($word)}};
			foreach my $lang (@langs) {
#print STDERR "$word (".$lang->[0].", ".$lang->[1].")\n";
				$score{$lang->[0]} += 1/@langs; # $lang->[1];
			}
		}
		$words++;
	}
	if ($words > 0) {
		my $lang = $self->{'defaultLanguage'} ne '' ? $self->{'defaultLanguage'} : '?';
		my $max = exists $score{$lang} ? $score{$lang} : -1;
		foreach my $k (keys %score) {
			if ($score{$k} > $max) {
				$max = $score{$k};
				$lang = $k;
			}
		}
		push(@lang, $lang);
	}

	$tr->SetLanguage(join(" ", @lang));
	
	$self->{'items'}++;
	return $resource;
}

1;
