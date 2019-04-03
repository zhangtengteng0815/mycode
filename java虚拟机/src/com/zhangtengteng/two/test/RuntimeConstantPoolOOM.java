package com.zhangtengteng.two.test;

import java.util.ArrayList;
import java.util.List;

/**
 * -XX:PermSize=10M -XX:MaxPermSize=10M
 * @author user
 *
 */
public class RuntimeConstantPoolOOM {
	public static void main(String[] args) {
		//使用List保持常量池引用，避免Full GC回收常量池行为
		List<String> list=new ArrayList<String>();
		//10M的PermSize在integer范围内足够产生OOM了
		int i=0;
		//String.intern()这个方法作用是：如果池中已经包含一个等于此String对象的字符串，则返回代表池中这个字符串的String对象
		//否则，将此String对象包含的字符串添加到常量池中，并且返回此String对象的引用。
		while(true){
			list.add(String.valueOf(i++).intern());
		}
	}
}
