package com.zhangtengteng.one;

import java.awt.BorderLayout;
import java.awt.Font;

import javax.swing.JApplet;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.SwingConstants;

public class WelcomeApplet extends JApplet{
	public void init(){
		setLayout(new BorderLayout());
		JLabel label=new JLabel(getParameter("greeting"),SwingConstants.CENTER);
		label.setFont(new Font("Serif", Font.BOLD, 18));
		add(label);
	
	}
}
