# .bashrc

# Source global definitions
if [ -f /etc/bashrc ]; then
	. /etc/bashrc
fi

# Uncomment the following line if you don't like systemctl's auto-paging feature:
# export SYSTEMD_PAGER=
#export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/brh22/dev_societies/societies/lib/

# User specific aliases and functions
alias T="cd /home/brh22/testing; ls -la"
alias R="cd /home/brh22/societies2017/_Results; ls -la"
alias C="cd /home/brh22/societies2017/Configs; ls -la"
alias P="cd /home/brh22/societies2017/PBS_scripts; ls -la"
alias rmlogs="rm *pbs.*; ll"
alias S="cd /home/brh22/societies2017"
alias jg="cd /home/jgsherw/"
