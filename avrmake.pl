#!/usr/bin/perl
use strict;
use warnings;
use autodie;

use constant {
    CMD_COMPILE => 'avr-gcc -mmcu=%s -Os -c -DF_CPU=%dUL %s.c -o %s.o',
    CMD_ELF     => 'avr-gcc -mmcu=%s %s.o -o %s.elf',
    CMD_HEX     => 'avr-objcopy -O ihex -R .eeprom %s.elf %s.hex',
    CMD_WRITE   => 'avrdude -p %s -c %s -U flash:w:%s.hex',
};

my %config = (
    home => {
        mmcu       => 'atmega8',
        DF_CPU     => 1000000,
        programmer => 'usbasp',
    },
    uj => {
        mmcu       => 'atmega32',
        DF_CPU     => 16000000,
        programmer => 'avrispmkII',
    },
);

my ($mode, $target) = @ARGV;
my @commands = (
    sprintf(CMD_COMPILE, @{ $config{$mode} }{qw(mmcu DF_CPU)}, ($target) x 2),
    sprintf(CMD_ELF, $config{$mode}{mmcu}, ($target) x 2),
    sprintf(CMD_HEX, ($target) x 2),
    sprintf(CMD_WRITE, @{ $config{$mode} }{qw(mmcu programmer)}, $target),
);

foreach my $command (@commands) {
    system $command and die;
}
