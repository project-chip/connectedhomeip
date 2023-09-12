if [[ $SHELL == "/bin/zsh" ]]; then
  echo "idt using zsh config"
  export IDT_SRC_PARENT="$(dirname $0)/../.."
else
  echo "idt using bash config"
  export IDT_SRC_PARENT="$(dirname ${BASH_SOURCE[0]:-$0})/../.."
fi

export IDT_OUTPUT_DIR="IDT_ARTIFACTS"

alias idt_dir="echo \"idt dir $IDT_SRC_PARENT\""
idt_dir
alias idt_go="cd \"$IDT_SRC_PARENT\""

alias idt_activate="idt_go && source idt/scripts/activate.sh"
alias idt_bootstrap="idt_go && source idt/scripts/bootstrap.sh"
alias idt_build="idt_go && source idt/scripts/build.sh"
alias idt_clean="idt_go && source idt/scripts/clean.sh"
alias idt_connect="idt_go && source idt/scripts/connect.sh"
alias idt_fetch_artifacts="idt_go && source idt/scripts/fetch_artifacts.sh"
alias idt_prune_docker="idt_go && source idt/scripts/prune_docker.sh"
alias idt_push="idt_go && source idt/scripts/push.sh"
alias idt_vars="idt_go && source idt/scripts/vars.sh"
alias idt_clean_artifacts="idt_go && source idt/scripts/clean_artifacts.sh"

alias idt="idt_go && python3 idt "

echo "idt commands available! type idt and press tab twice to see available commands."
