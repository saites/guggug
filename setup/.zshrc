autoload -U compinit
compinit

alias ls="ls  --color=auto -F" # color ls is cool
source ~/.zsh_files/.zsh.prompt  # set the prompt

# sometimes I like to keep a history
export SAVEHIST=2000
export HISTFILE="$HOME/.zsh_files/.zsh.history"
export SHELL=/usr/local/bin/zsh
export EDITOR=vim
export VISUAL=vim
export CVS_RSH=ssh
umask 077

# source my alises
if [ -f $HOME/.zsh_files/.zsh.aliases ] ; then
  source $HOME/.zsh_files/.zsh.aliases
fi

limit coredumpsize 0    # Turn off core dumps

export LESS='-ifqm'     # set up the parameters for 'less'
export PAGER=less       # use less for stuff (such as man)

# set some usefull options
setopt NO_HUP           # make jobs not die when terminal is closed
setopt NO_BEEP          # don't beep at me all the time

