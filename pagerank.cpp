#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <cassert>
#include <vector>
#include <string>
#include <algorithm>

#include<cstring>
#include<stdlib.h>
#include<stdio.h>

using namespace std;

vector<int> allWebsite;

struct WebData
{
	int WebID;
	int NumOfOut;
	vector<int> Target;
};

struct Score
{
	int WebID;
	double score;
};

//建立一个以WebData为类型vector一维数组
vector<WebData> Data;

//4x4的Map拆分
vector<WebData> Map[4][4];

//二分查找
int BinSearch(vector<int> R, int n, int Target)
{
	//在有序表R[0,n-1]中进行二分查找，成功时返回结点的位置，失败时返回-1

	int low = 0, high = n - 1, mid;     //置当前查找区间上、下界的初值

	while (low <= high)
	{
		if (R[low] == Target)
			return low;

		if (R[high] == Target)
			return high;         

		mid = low + ((high - low) / 2);

		if (R[mid] == Target)
			return mid;             //查找成功返回

		if (R[mid] < Target)
			low = mid + 1;              //继续在R[mid+1..high]中查找
		else
			high = mid - 1;             //继续在R[low..mid-1]中查找
	}

	//当low>high时表示所查找区间内没有结果，查找失败
	if (low>high)
		return -1;
}

//按照源网页号码升序排列
bool cmp2(WebData a, WebData b)
{
	return a.WebID < b.WebID;
}

//按照pagerank值降序排列
bool cmpPGRK(Score a, Score b)
{
	return a.score > b.score;
}

/*  Linux（Ubuntu）下运行则取消注释
int stoi(string string)
{
	char *end;
	char temp[100];
	strcpy(temp, string.c_str());
	int Targ = strtol(temp, &end, 10);
	return Targ;
}
*/

//读入数据（优化矩阵）：转移矩阵的紧凑表示法
void load()
{
	//打开wikidata.txt
	ifstream infile1;
	infile1.open("wikidata.txt");
	assert(infile1.is_open());   //若失败,则输出错误消息,并终止程序运行 


	/*————向Data数组内存数据————*/

	string s; //存放读取行数据
	string blank = "	";
	int Web = -1;
	vector<int> TemporaryTarget;

	while (1)//逐行读取数据
	{
		getline(infile1, s);

		if (s == "") //已经跑到最后了，要把最后一个数据存入
		{
			WebData save;
			save.WebID = Web;
			save.NumOfOut = TemporaryTarget.size();
			save.Target = TemporaryTarget;
			Data.push_back(save);

			break;
		}


		int LocBlack = s.find(blank); //空格所在位置

		 //获取当前源网页
		string NowWeb = s.substr(0, LocBlack);
		int Now_Web = stoi(NowWeb);


		//如果当前源网页和上一个网页号码不一样，则是新的网页
		if (Now_Web != Web)
		{
			//如果现在Data数组不是空的，对上一个源网页的收尾操作，存入Data中
			if (Web != -1)
			{
				WebData save;
				save.WebID = Web;
				save.NumOfOut = TemporaryTarget.size();
				save.Target = TemporaryTarget;
				Data.push_back(save);
			}


			//对相关数据进行重新初始化
			TemporaryTarget.clear();
			Web = Now_Web; //更新当前源网页

			//向临时目标数组中存入第一个数据
			int leng = s.length();
			string Tar = s.substr(LocBlack + 1, leng - LocBlack);

			int Targ = stoi(Tar); //第一个数据
			TemporaryTarget.push_back(Targ); //存入

			continue;
		}

		//如果当前源网页和上一个网页号码一样，则继续对上一个网页的操作
		if (Now_Web == Web)
		{
			int leng = s.length();
			string Tar = s.substr(LocBlack + 1, leng - LocBlack);

			int Targ = stoi(Tar); //获取目标网页的号码
			TemporaryTarget.push_back(Targ); //存入

			continue;
		}

	}

	infile1.close();

}

//把所有当过目标的网页集中表示
vector<int> AllTarget()
{
	vector<int> Mrow;

	for (int i = 0; i < Data.size(); i++) //外层遍历所有源网页
	{
		for (int j = 0; j < Data[i].Target.size(); j++) //内层遍历当前源网页的目标网页
		{
			int now = Data[i].Target[j];

			int nPosition = BinSearch(Mrow,Mrow.size(),now);

			if (nPosition == -1) //当前目标网页在Mrow中不存在，则存入
			{
				Mrow.push_back(now);
				sort(Mrow.begin(), Mrow.end(), less<int>());
			}
		}
	}

	return Mrow;
}

//求所有源网页、目标网页的并集
vector<int> AllWeb(vector<int> AllTarget)
{
	//先把所有目标网页赋给AllWebsite
	vector<int> AllWebsite = AllTarget;

	//遍历所有源网页，如果有是源网页但是不是目标网页的，插入
	for (int i = 0; i < Data.size(); i++)
	{
		int now = Data[i].WebID;

		int nPosition = BinSearch(AllWebsite, AllWebsite.size(), now);

		if (nPosition == -1) //当前目标网页在Mrow中不存在，则存入
		{
			AllWebsite.push_back(now);
			sort(AllWebsite.begin(), AllWebsite.end(), less<int>());
		}

	}

	return AllWebsite;
}

//分块矩阵：把Map拆分成4x4=16个Map
void fourXfour_Map()
{
	//总网页数=6110(0-6109),把他们分成4份

	/* 分组：以下数字均为Data下标，不是源网页码
	A：0-1526 （1527个数据）
	B：1527-3053  （1527个）
	C：3054-4580  （1527个）
	D：4581-6109  （1529个）

	【4x4】 Map
	A   B   C   D
	A  00  01  02  03
	B  10  11  12  13
	C  20  21  22  23
	D  30  31  32  33
	*/


	/*——————AA，存入Map[0][0]——————*/
	int start1 = 0, end1 = 1526;
	int start2 = 0, end2 = 1526;

	int StartID2 = Data[start2].WebID;
	int EndID2 = Data[end2].WebID;

	for (int i = start1; i <= end1; i++)
	{
		//临时变量save
		WebData save;
		save.WebID = Data[i].WebID;
		save.NumOfOut = Data[i].NumOfOut;

		for (int j = 0; j < Data[i].NumOfOut; j++)
		{
			if (Data[i].Target[j] >= StartID2 && Data[i].Target[j] <= EndID2)
				save.Target.push_back(Data[i].Target[j]);
		}

		if (save.Target.size() != 0)
			Map[0][0].push_back(save);
	}
	/*————————————————————*/

	/*——————AB，存入Map[0][1]——————*/
	start1 = 0, end1 = 1526;
	start2 = 1527, end2 = 3053;

	StartID2 = Data[start2].WebID;
	EndID2 = Data[end2].WebID;

	for (int i = start1; i <= end1; i++)
	{
		//临时变量save
		WebData save;
		save.WebID = Data[i].WebID;
		save.NumOfOut = Data[i].NumOfOut;

		for (int j = 0; j < Data[i].NumOfOut; j++)
		{
			if (Data[i].Target[j] >= StartID2 && Data[i].Target[j] <= EndID2)
				save.Target.push_back(Data[i].Target[j]);
		}

		if (save.Target.size() != 0)
			Map[0][1].push_back(save);
	}
	/*————————————————————*/

	/*——————AC，存入Map[0][2]——————*/
	start1 = 0, end1 = 1526;
	start2 = 3054, end2 = 4580;

	StartID2 = Data[start2].WebID;
	EndID2 = Data[end2].WebID;

	for (int i = start1; i <= end1; i++)
	{
		//临时变量save
		WebData save;
		save.WebID = Data[i].WebID;
		save.NumOfOut = Data[i].NumOfOut;

		for (int j = 0; j < Data[i].NumOfOut; j++)
		{
			if (Data[i].Target[j] >= StartID2 && Data[i].Target[j] <= EndID2)
				save.Target.push_back(Data[i].Target[j]);
		}

		if (save.Target.size() != 0)
			Map[0][2].push_back(save);
	}
	/*————————————————————*/

	/*——————AD，存入Map[0][3]——————*/
	start1 = 0, end1 = 1526;
	start2 = 4581, end2 = 6109;

	StartID2 = Data[start2].WebID;
	EndID2 = Data[end2].WebID;

	for (int i = start1; i <= end1; i++)
	{
		//临时变量save
		WebData save;
		save.WebID = Data[i].WebID;
		save.NumOfOut = Data[i].NumOfOut;

		for (int j = 0; j < Data[i].NumOfOut; j++)
		{
			if (Data[i].Target[j] >= StartID2 && Data[i].Target[j] <= EndID2)
				save.Target.push_back(Data[i].Target[j]);
		}

		if (save.Target.size() != 0)
			Map[0][3].push_back(save);
	}
	/*————————————————————*/

	/*——————BA，存入Map[1][0]——————*/
	start1 = 1527, end1 = 3053;
	start2 = 0, end2 = 1526;

	StartID2 = Data[start2].WebID;
	EndID2 = Data[end2].WebID;

	for (int i = start1; i <= end1; i++)
	{
		//临时变量save
		WebData save;
		save.WebID = Data[i].WebID;
		save.NumOfOut = Data[i].NumOfOut;

		for (int j = 0; j < Data[i].NumOfOut; j++)
		{
			if (Data[i].Target[j] >= StartID2 && Data[i].Target[j] <= EndID2)
				save.Target.push_back(Data[i].Target[j]);
		}

		if (save.Target.size() != 0)
			Map[1][0].push_back(save);
	}
	/*————————————————————*/

	/*——————BB，存入Map[1][1]——————*/
	start1 = 1527, end1 = 3053;
	start2 = 1527, end2 = 3053;

	StartID2 = Data[start2].WebID;
	EndID2 = Data[end2].WebID;

	for (int i = start1; i <= end1; i++)
	{
		//临时变量save
		WebData save;
		save.WebID = Data[i].WebID;
		save.NumOfOut = Data[i].NumOfOut;

		for (int j = 0; j < Data[i].NumOfOut; j++)
		{
			if (Data[i].Target[j] >= StartID2 && Data[i].Target[j] <= EndID2)
				save.Target.push_back(Data[i].Target[j]);
		}

		if (save.Target.size() != 0)
			Map[1][1].push_back(save);
	}
	/*————————————————————*/

	/*——————BC，存入Map[1][2]——————*/
	start1 = 1527, end1 = 3053;
	start2 = 3054, end2 = 4580;

	StartID2 = Data[start2].WebID;
	EndID2 = Data[end2].WebID;

	for (int i = start1; i <= end1; i++)
	{
		//临时变量save
		WebData save;
		save.WebID = Data[i].WebID;
		save.NumOfOut = Data[i].NumOfOut;

		for (int j = 0; j < Data[i].NumOfOut; j++)
		{
			if (Data[i].Target[j] >= StartID2 && Data[i].Target[j] <= EndID2)
				save.Target.push_back(Data[i].Target[j]);
		}

		if (save.Target.size() != 0)
			Map[1][2].push_back(save);
	}
	/*————————————————————*/

	/*——————BD，存入Map[1][3]——————*/
	start1 = 1527, end1 = 3053;
	start2 = 4581, end2 = 6109;

	StartID2 = Data[start2].WebID;
	EndID2 = Data[end2].WebID;

	for (int i = start1; i <= end1; i++)
	{
		//临时变量save
		WebData save;
		save.WebID = Data[i].WebID;
		save.NumOfOut = Data[i].NumOfOut;

		for (int j = 0; j < Data[i].NumOfOut; j++)
		{
			if (Data[i].Target[j] >= StartID2 && Data[i].Target[j] <= EndID2)
				save.Target.push_back(Data[i].Target[j]);
		}

		if (save.Target.size() != 0)
			Map[1][3].push_back(save);
	}
	/*————————————————————*/

	/*——————CA，存入Map[2][0]——————*/
	start1 = 3054, end1 = 4580;
	start2 = 0, end2 = 1526;

	StartID2 = Data[start2].WebID;
	EndID2 = Data[end2].WebID;

	for (int i = start1; i <= end1; i++)
	{
		//临时变量save
		WebData save;
		save.WebID = Data[i].WebID;
		save.NumOfOut = Data[i].NumOfOut;

		for (int j = 0; j < Data[i].NumOfOut; j++)
		{
			if (Data[i].Target[j] >= StartID2 && Data[i].Target[j] <= EndID2)
				save.Target.push_back(Data[i].Target[j]);
		}

		if (save.Target.size() != 0)
			Map[2][0].push_back(save);
	}
	/*————————————————————*/

	/*——————CB，存入Map[2][1]——————*/
	start1 = 3054, end1 = 4580;
	start2 = 1527, end2 = 3053;

	StartID2 = Data[start2].WebID;
	EndID2 = Data[end2].WebID;

	for (int i = start1; i <= end1; i++)
	{
		//临时变量save
		WebData save;
		save.WebID = Data[i].WebID;
		save.NumOfOut = Data[i].NumOfOut;

		for (int j = 0; j < Data[i].NumOfOut; j++)
		{
			if (Data[i].Target[j] >= StartID2 && Data[i].Target[j] <= EndID2)
				save.Target.push_back(Data[i].Target[j]);
		}

		if (save.Target.size() != 0)
			Map[2][1].push_back(save);
	}
	/*————————————————————*/

	/*——————CC，存入Map[2][2]——————*/
	start1 = 3054, end1 = 4580;
	start2 = 3054, end2 = 4580;

	StartID2 = Data[start2].WebID;
	EndID2 = Data[end2].WebID;

	for (int i = start1; i <= end1; i++)
	{
		//临时变量save
		WebData save;
		save.WebID = Data[i].WebID;
		save.NumOfOut = Data[i].NumOfOut;

		for (int j = 0; j < Data[i].NumOfOut; j++)
		{
			if (Data[i].Target[j] >= StartID2 && Data[i].Target[j] <= EndID2)
				save.Target.push_back(Data[i].Target[j]);
		}

		if (save.Target.size() != 0)
			Map[2][2].push_back(save);
	}
	/*————————————————————*/

	/*——————CD，存入Map[2][3]——————*/
	start1 = 3054, end1 = 4580;
	start2 = 4581, end2 = 6109;

	StartID2 = Data[start2].WebID;
	EndID2 = Data[end2].WebID;

	for (int i = start1; i <= end1; i++)
	{
		//临时变量save
		WebData save;
		save.WebID = Data[i].WebID;
		save.NumOfOut = Data[i].NumOfOut;

		for (int j = 0; j < Data[i].NumOfOut; j++)
		{
			if (Data[i].Target[j] >= StartID2 && Data[i].Target[j] <= EndID2)
				save.Target.push_back(Data[i].Target[j]);
		}

		if (save.Target.size() != 0)
			Map[2][3].push_back(save);
	}
	/*————————————————————*/

	/*——————DA，存入Map[3][0]——————*/
	start1 = 4581, end1 = 6109;
	start2 = 0, end2 = 1526;

	StartID2 = Data[start2].WebID;
	EndID2 = Data[end2].WebID;

	for (int i = start1; i <= end1; i++)
	{
		//临时变量save
		WebData save;
		save.WebID = Data[i].WebID;
		save.NumOfOut = Data[i].NumOfOut;

		for (int j = 0; j < Data[i].NumOfOut; j++)
		{
			if (Data[i].Target[j] >= StartID2 && Data[i].Target[j] <= EndID2)
				save.Target.push_back(Data[i].Target[j]);
		}

		if (save.Target.size() != 0)
			Map[3][0].push_back(save);
	}
	/*————————————————————*/

	/*——————DB，存入Map[3][1]——————*/
	start1 = 4581, end1 = 6109;
	start2 = 1527, end2 = 3053;

	StartID2 = Data[start2].WebID;
	EndID2 = Data[end2].WebID;

	for (int i = start1; i <= end1; i++)
	{
		//临时变量save
		WebData save;
		save.WebID = Data[i].WebID;
		save.NumOfOut = Data[i].NumOfOut;

		for (int j = 0; j < Data[i].NumOfOut; j++)
		{
			if (Data[i].Target[j] >= StartID2 && Data[i].Target[j] <= EndID2)
				save.Target.push_back(Data[i].Target[j]);
		}

		if (save.Target.size() != 0)
			Map[3][1].push_back(save);
	}
	/*————————————————————*/

	/*——————DC，存入Map[3][2]——————*/
	start1 = 4581, end1 = 6109;
	start2 = 3054, end2 = 4580;

	StartID2 = Data[start2].WebID;
	EndID2 = Data[end2].WebID;

	for (int i = start1; i <= end1; i++)
	{
		//临时变量save
		WebData save;
		save.WebID = Data[i].WebID;
		save.NumOfOut = Data[i].NumOfOut;

		for (int j = 0; j < Data[i].NumOfOut; j++)
		{
			if (Data[i].Target[j] >= StartID2 && Data[i].Target[j] <= EndID2)
				save.Target.push_back(Data[i].Target[j]);
		}

		if (save.Target.size() != 0)
			Map[3][2].push_back(save);
	}
	/*————————————————————*/

	/*——————DD，存入Map[3][3]——————*/
	start1 = 4581, end1 = 6109;
	start2 = 4581, end2 = 6109;

	StartID2 = Data[start2].WebID;
	EndID2 = Data[end2].WebID;

	for (int i = start1; i <= end1; i++)
	{
		//临时变量save
		WebData save;
		save.WebID = Data[i].WebID;
		save.NumOfOut = Data[i].NumOfOut;

		for (int j = 0; j < Data[i].NumOfOut; j++)
		{
			if (Data[i].Target[j] >= StartID2 && Data[i].Target[j] <= EndID2)
				save.Target.push_back(Data[i].Target[j]);
		}

		if (save.Target.size() != 0)
			Map[3][3].push_back(save);
	}
	/*————————————————————*/

}

//Map
vector< vector<double> > CalcuMap(vector<WebData> SourceMap, vector<Score> pagerank, vector< vector<double> > MapValue)
{
	int nPosition = 0;

	//相关数据存入
	for (int i = 0; i < SourceMap.size(); i++) //遍历所有源网页
	{
		//NowScore = pagerank[源网页].score / 源网页的出度数

		//遍历PageRank查找当前源网页，取出它的位置，得到score
		
		for (; nPosition < pagerank.size(); nPosition++)
		{
			if (pagerank[nPosition].WebID == SourceMap[i].WebID)
				break;
		}

		for (int j = 0; j < SourceMap[i].Target.size(); j++)  //每一源网页对应的所有目标网页
		{
			double NowScore = pagerank[nPosition].score / SourceMap[i].NumOfOut;

			//这里号码与位置不是一一对应的关系，所以要先获取SourceMap[i].Target[j]应该处于的位置
			int position = BinSearch(allWebsite, allWebsite.size(), SourceMap[i].Target[j]);

			MapValue[ position ].push_back(NowScore);

		}
	}

	return MapValue;

}

//Reduce
vector<Score> CalcuReduce(vector< vector<double> > MapValue, vector<Score> pagerank)
{
	int rowNum = pagerank.size();

	vector<Score> NewPageRank(rowNum);

	//遍历PageRank中的每一个网页，填写新的PageRank值
	//遍历MapValue，搜集该目标网页的所有分数

	for (int k = 0; k < MapValue.size(); k++) 
	{		
		int Name = pagerank[k].WebID;

		double sco = 0;

		for (int i = 0; i < MapValue[k].size(); i++)
			sco += MapValue[k][i];

		//考虑dead ends和spider trap的计算方法
		double TrueSco = 0.85*sco + 0.15 / rowNum;
		
		NewPageRank[k].WebID = pagerank[k].WebID;
		NewPageRank[k].score = TrueSco;
	}
	
	return NewPageRank;
}

//总计算 Map_reduce
vector<Score> Map_reduce(vector<WebData> SourceMap[][4], vector<Score> PageRank)
{
	//执行100次，保证其收敛性
	for (int k = 0; k < 100; k++)
	{
		int rowNum = PageRank.size();

		//16个分块进行map操作
		vector< vector<double> > MapValue(rowNum);
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				MapValue = CalcuMap(SourceMap[i][j], PageRank, MapValue);
			}
		}

		//reduce操作
		PageRank = CalcuReduce(MapValue, PageRank);

	}

	sort(PageRank.begin(), PageRank.end(), cmpPGRK); //排序

	return PageRank;
}

//初始化PageRank
vector<Score> InitPageRank(vector<int> allWebsite, int num, double score)
{
	vector<Score> PageRank(num);  //PageRank数组
	for (int i = 0; i < num; i++)
	{
		PageRank[i].WebID = allWebsite[i];
		PageRank[i].score = score;
	}
	return PageRank;
}

//写结果
void WriteResult(vector<Score> PageRank)
{
	fstream file1;
	file1.open("result.txt");
	for (int i = 0; i < 100; i++)
	{
		file1 <<"["<< PageRank[i].WebID <<"]"<< "   " <<"["<< PageRank[i].score <<"]"<< endl;
	}
	file1.close();
}

int main()
{
	load();
	cout << "数据wikidata已读取完毕" << endl;

	sort(Data.begin(), Data.end(), cmp2);
	fourXfour_Map();
	cout << "已分块完毕" << endl;

	vector<int> allTarget = AllTarget();
	allWebsite = AllWeb(allTarget);
	cout << "所有网页已获取完毕" << endl;

	int num = allWebsite.size();
	double score = 1.0 / num;
	vector<Score> PageRank = InitPageRank(allWebsite, num, score); 
	cout << "PageRank已初始化完毕" << endl;

	PageRank = Map_reduce(Map, PageRank);
	cout << "Map_reduce已执行完毕" << endl;

	WriteResult(PageRank);
	cout << "结果已写入完毕" << endl;
	
	return 0;
}

