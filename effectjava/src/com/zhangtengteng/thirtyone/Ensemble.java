package com.zhangtengteng.thirtyone;

public enum Ensemble {
	SOLO, DUET, TRIO, QUARTET, QUINTET,
	SEXTET, SEPTET ,OCTET ,NONET ,DECTET;
	public int numberOfMusicians(){
		return ordinal()+1;
	}
}
