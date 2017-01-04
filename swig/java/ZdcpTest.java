/* ----------------------------------------------------------------------------
 * This file is a simple demo for zdcp interface usage.
 * Version: 1.0.0
 * Author:wanyu
*/

public class ZdcpTest{
	static {
		System.loadLibrary("zdcpjava");
	}
	public static void main(String[] args){
		System.out.println(System.getProperty("java.library.path"));
		SWIGTYPE_p_void handle = zdcp.zdcp_create("../../config.cfg");
		System.out.println(handle);
		System.out.println(zdcp.zdcp_error());
		String doc="<TitleWords: good><BodyWords: tv mobile ><category: >";
		doc="<TitleWords: good><BodyWords: bad  ><category: >";

		
		int i ;
		if((i=zdcp.zdcp_classify(handle,doc,null))>0){
			System.out.println((doc+":"+i+":"+zdcp.zdcp_getclassname(handle,i)));
		}else{
			System.out.println(doc+":fail to find suitable catagory.");
		}

	}
}


