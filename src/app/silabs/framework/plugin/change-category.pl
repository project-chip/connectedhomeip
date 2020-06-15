#!/usr/bin/perl -w

################################################################################
# See $USAGE below for description.

################################################################################
# Modules

use strict;
use Getopt::Long;
use File::Basename;

################################################################################
# Globals

my $THIS = $0; $THIS =~ s%.*/%%;

my $USAGE = <<END_OF_TEXT;

  $THIS [ parameters ]

  This script updates plugin.properties files for the specified plugins to
  change their 'category' property accordingly.

  -d, --directory <plugin>
    The directory containing the 'plugin.properties' file you want to update.
    Multiple '-d' options allowed.

  -c, --category <name>
    The new category text.

  -p, --print <directory>
    Find all 'plugin.properties' files in all sub-directories and print
    out the categories that they belong to.

  -r, --real-name
    Print the plugin's formal name instead of the directory name.

  -f, --filter <category>
    Filter printing and display only the specified category.

  -h, --help
     Print this usage

END_OF_TEXT

my $PROPERTIES_FILE = "plugin.properties";

################################################################################
# Functions

exit Main();

sub Main
{
	my $FilterCategory;
	my @Directories;
	my $PrintDir;
	my $Category;
	my $RealName;
  Getopt::Long::config("bundling");
  GetOptions
  (
	 'd|directory=s' => \@Directories,
	 'c|category=s'  => \$Category,
	 'p|print=s'     => \$PrintDir,
	 'f|filter=s'    => \$FilterCategory,
	 'r|real-name'   => \$RealName,
	) or die $USAGE;

	die "Error: Must specify '-d' or '-p'.\n"
		unless ($PrintDir || scalar(@Directories));
	die "Error: Cannot specify both '-d' and '-p'.\n"
		if ($PrintDir && scalar(@Directories));
	die "Error: Must specify '-c' option with '-d'.\n"
		if (scalar(@Directories) && !$Category);

	if ($PrintDir) {
		die "Error: Directory '$PrintDir' does not exist.\n" unless (-e $PrintDir);
		return PrintPlugins($PrintDir, $FilterCategory, $RealName);
	} else {
		return ChangePlugins($Category, @Directories);
	}

	return 0;
}

sub ChangePlugins
{
	my $Category = shift(@_);
	my @Directories = @_;

	foreach my $Dir ( sort @Directories ) {
		$Dir =~ s%/$%%;  # remove trailing slash
		my $File = $Dir . "/" . $PROPERTIES_FILE;

		die "Error: Plugin directory '$Dir' is invalid.\n" unless (-e $File);
		my $Status = open(INPUT, "$File");
		die "Error: Could not open '$File': $!\n" unless $Status;
		my @Data = <INPUT>;
		close INPUT;
		my $CategoryReplaced = 0;
		for (my $i = 0; $i < scalar(@Data); $i++) {
			if (grep /^category=/, $Data[$i]) {
				$Data[$i] =~ s/=.*/=$Category/;
				$CategoryReplaced = 1;
			}
		}
		$Status = open(OUTPUT, ">$File");
		die "Error: Could not open file '$File' for writing: $!\n" unless $Status;
		my $AdditionalLine;
		foreach my $Line ( @Data ) {
			unless ($CategoryReplaced) {
				if ($Line =~ /^name=/) {
					$AdditionalLine = "category=$Category\n";
				}
			}
			print OUTPUT $Line;
			print OUTPUT $AdditionalLine if $AdditionalLine;
			$AdditionalLine = "";
		}
		close OUTPUT;
	}
	return 0;
}

sub PrintPlugins
{
	my ($PrintDir, $FilterCategory, $RealName) = @_;
	my @Output = `find $PrintDir -iname '$PROPERTIES_FILE'`;
	my $Status = $?;
	die "Error: Could not find properties files.\n" if ($Status);
	my %Categories;
	foreach my $File ( @Output ) {
		chomp $File;
		my $Dir = dirname($File);
		my $PluginName = $Dir;
		$PluginName =~ s%.*/%%;
		$Status = open(INPUT, "$File");
		die "Error: Could not open file '$File' for reading: $!\n" unless ($Status);
		my $Category = "Unknown";
		my $FormalName;
		while (my $Line = <INPUT>) {
			chomp $Line;
			if ($Line =~ /^category=(.*)/) {
				$Category=$1;
			}
			if ($Line =~ /^name=(.*)/) {
				$FormalName = $1;
			}
		}
		close INPUT;
		die "Error: Could not determine formal name for directory '$PluginName'.\n"
			unless ($FormalName);
		push @{$Categories{$Category}}, ($RealName ? $FormalName : $PluginName);
	}

	foreach my $Category ( sort keys %Categories ) {
		my $Number = scalar (@{$Categories{$Category}});
		next if ($FilterCategory && $Category ne $FilterCategory);
		print "$Category ($Number plugins)\n";
		foreach my $Plugin ( sort @{$Categories{$Category}} ) {
			print "  $Plugin\n";
		}
	}

	return 0;
}
