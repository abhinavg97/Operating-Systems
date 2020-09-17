#include<bits/stdc++.h>
#include<iostream>                  // the logic of this program is very similar to the EDF one, check that for comments
#include<string>
using namespace std;

struct node{
	int id;
	int p;
	int t;
	int k;
	int rem;
	int start;
};




void swap(struct node *temp1,struct node *temp2)
{
	struct node temp3 = *temp1;
	*temp1 = *temp2;
	*temp2 = temp3;
}

bool cmp(struct node temp1, struct node temp2) 
{
	return (temp1.p<temp2.p);
}


int n;
int np=0;
vector<struct node> q;
int miss=0;
int success;
int wait=0;

int main()
{

	ifstream in("inp-params.txt");
	ofstream out("RM-Log.txt");

	if(!out.is_open())
	{
		cout<<"Cannot open output file!\n"<<flush;
		exit(0);
	}

	if(in.is_open())
	{
		in>>n;
		
		struct node temp1;

		for(int i=0;i<n;++i)
		{
			in>>temp1.id>>temp1.t>>temp1.p>>temp1.k;
			np+=temp1.k;
			q.push_back(temp1);
			out<<"Process P"<<temp1.id<<": processing time="<<temp1.t<<"; deadline:"<<temp1.p<<"; period:"<<temp1.p<<" joined the system at time 0.\n"<<flush;
		}

		in.close();

		sort(q.begin(), q.end(),cmp);
		
		for(int i=0;i<n;++i)
			q[i].rem = 0;
		
		int prev=-1,curr;
		int flag2=0;

		for(int t=0;q.size();++t)
		{
			int flag=0;
			for(int j=0;j<n;++j)
				if(q[j].rem && t%q[j].p==0)
				{
					miss++;
					wait += (t - q[j].start) - (q[j].t - q[j].rem);
					q[j].rem = q[j].t;
					q[j].start = t;
					q[j].k--;
					out<<"Process P"<<q[j].id<<" missed its deadline at time "<<t<<".\n"<<flush;
					
					if(q[j].k==0) 
					{
						swap(&q[j],&q[n-1]);
						q.pop_back();
						n--;
						// prev=-1;
						sort(q.begin(),q.end(),cmp);
					}
				}
				else if(t%q[j].p==0)
				{
					q[j].start = t;
					q[j].rem = q[j].t;
				}
				
				else if(q[j].rem==0)
					flag++;
			
			if(flag==n)
			{
				flag2=1;
				continue;
			}
			else
			{
				if(flag2) 
					out<<"CPU is idle till time "<<t-1<<".\n"<<flush;
				flag2=0;
			}

			int i;

			for(i=0;i<n;++i)
				if(q[i].rem) break;
			
			curr = i;


			if(prev != curr)
			{
				if(prev==-1)
					out<<"Process P"<<q[curr].id<<" starts execution at time "<<t<<".\n"<<flush;

				else if( q[prev].rem==0 && q[curr].t==q[curr].rem) 
					out<<"Process P"<<q[curr].id<<" starts execution at time "<<t<<".\n"<<flush;

				else if(q[prev].rem==0 )
					out<<"Process P"<<q[curr].id<<" resumes execution at time "<<t<<".\n"<<flush;
				
				else
				{
					out<<"Process P"<<q[prev].id<<" is preempted by Process P"<<q[curr].id<<" at time "<<t<<". Remaining processing time:"<<q[prev].rem<<".\n"<<flush;
					out<<"Process P"<<q[curr].id<<" starts execution at time "<<t<<".\n"<<flush;
				}
				prev=curr;
			}
			q[curr].rem--;
			if(q[curr].rem==0)
			{ 
				out<<"Process P"<<q[curr].id<<" finishes execution at time "<<t<<".\n"<<flush;
				wait += t - q[curr].start - q[curr].t;
				q[curr].k--;
				prev=-1;
			}
			if(q[curr].k==0) 
			{
				swap(&q[curr],&q[n-1]);
				q.pop_back();
				n--;
				prev=-1;
				sort(q.begin(),q.end(),cmp);
			}
		}
		success=np-miss;
		float avgwait = (wait*1.0)/np;
		out.close();

		ofstream out("RM-Stats.txt");
		out<<"Total number of processes that came into the system are "<<np<<".\n"<<flush;
		out<<"Number of processes that successfully completed are "<<success<<".\n"<<flush;
		out<<"Number of processes that missed the deadline are "<<miss<<".\n"<<flush;
		out<<"Average waiting time for the algorith is "<<avgwait<<".\n"<<flush;
		out.close();
	}

	else 
		cout<<"Error opening the file!\n"<<flush;
		
	return 0;
}
