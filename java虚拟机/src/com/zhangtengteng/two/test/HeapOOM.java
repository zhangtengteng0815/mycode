package com.zhangtengteng.two.test;

import java.util.ArrayList;
import java.util.List;

/**
 * VM Args：-Xms20m -Xmx20m -XX:+HeapDumpOnOutOfMemoryError
 * @author user
 *
 */
public class HeapOOM {
	static class OOMObject{	
	}
	public static void main(String[] args) {
		List<OOMObject> list=new ArrayList<OOMObject>();
		while(true){
			list.add(new OOMObject());
		}
	}
}
