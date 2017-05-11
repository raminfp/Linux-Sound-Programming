
import javax.swing.*;
import java.awt.*;

public class Controls extends JPanel {

    public Controls() {
	setLayout(new GridLayout(4, 2));
	add(new JLabel("Mic Volume"));
	JSpinner micVol = new JSpinner();
	add(micVol);

	add(new JLabel("Backing Volume"));
	JSpinner backingVol = new JSpinner();
	add(backingVol);

	add(new JLabel("Speed"));
	JSpinner speed = new JSpinner();
	add(speed);

	add(new JLabel("Pitch"));
	JSpinner pitch = new JSpinner();
	add(pitch);
    }
}