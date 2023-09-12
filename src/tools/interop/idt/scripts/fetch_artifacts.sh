source idt/scripts/vars.sh
idt_go
scp -r $PIUSER@$PIHOST:/home/$PIUSER/idt/$IDT_OUTPUT_DIR .
cd $IDT_OUTPUT_DIR
ls
