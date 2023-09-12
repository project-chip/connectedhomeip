source idt/scripts/vars.sh
mv idt/venv TEMPvenv
scp -r ./idt/* $PIUSER@$PIHOST:/home/$PIUSER/idt
mv TEMPvenv idt/venv
