package com.zhangtengteng.thirtytwo;

import java.util.EnumSet;
import java.util.Set;

public class Text {
	public enum Style{BOLD, ITALIC, UNDERLINE, STRIKETHROUGH}
	public void applyStyles(Set<Style> styles){
		System.out.println(styles);
		//....
	}
	public static void main(String[] args) {
		Text t=new Text();
		t.applyStyles(EnumSet.of(Style.BOLD,Style.ITALIC));
	}
}
