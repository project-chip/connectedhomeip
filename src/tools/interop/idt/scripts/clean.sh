source idt/scripts/vars.sh
if [[ "$(hostname)" == "$PIHOST" ]]; then
  echo "Target env detected, cleaning"
  sudo rm -R idt
else
  echo "Not in the target env, so not cleaning"
fi
