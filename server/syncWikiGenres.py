#!usr/bin/python
from bs4 import BeautifulSoup
import urllib2
import sys
import psycopg2
import os
import re

flog = open("wikiGenresLog.txt","w")

base_url = 'https://en.wikipedia.org/wiki/List_of_radio_stations_in_'
states = [('AK','Alaska'), ('AL','Alabama'), ('AR','Arkansas'),('AZ','Arizona'), ('CA','California'),('CO','Colorado'), \
	('CT','Connecticut'),('DE','Delaware'),('FL','Florida'),('GA','Georgia'),('HI','Hawaii'), ('IA','Iowa'),('ID','Idaho'), \
	('IL','Illinois'),('IN','Indiana'),('KS','Kansas'),('KY','Kentucky'),('LA','Louisiana'), ('MA','Massachusetts'),  \
	('MD','Maryland'),('ME','Maine'),('MI','Michigan'),('MN','Minnesota'),('MO','Missouri'),('MS','Mississippi'), \
	('MT','Montana'),('NC','North_Carolina'),('ND','North_Dakota'),('NE','Nebraska'),('NH','New_Hampshire'),('NJ','New_Jersey'),('NM','New_Mexico'), \
	('NV','Nevada'),('NY','New_York'),('OH','Ohio'),('OK','Oklahoma'),('OR','Oregon'),('PA','Pennsylvania'),('RI','Rhode_Island'), \
	('SC','South_Carolina'),('SD','South_Dakota'),('TN','Tennessee'),('TX','Texas'),('UT','Utah'),('VA','Virginia'),('VT','Vermont'), \
	('WA','Washington'),('WI','Wisconsin'),('WV','West_Virginia'),('WY','Wyoming'),('DC','Washington,_D.C.')]

genre_nicknames = ['country','jazz','classical','club','hip-hop','rnb','rock','oldies','christian','alternative','pop','talk','foreign'];

genre_keywords = [
	['folk',' blue','blue-grass','country','country','country','country','country','country'], \
	['jazz','ragtime','swing','funk','jazz','jazz','jazz','jazz','jazz'], \
	['classical','chamber','orchestra','symphony','instrumental','concert','classical','classical','classical'], \
	['dance','club','electr','disco','dubstep','step','dance','trance','house'], \
	['hip','urban','hip-hop','rap','dj','hip','hip','hip','hip'], \
	['rnb','r&b','soul','blues','rnb','rnb','rnb','rnb','rnb'], \
	['rock','rock&roll','rock and roll','metal','grunge','punk','80s','70s','60s'], \
	['oldies','50s','classic','40s','20s','30s','oldies','oldies','oldies'], \
	['christian','inspirational','gospel','christ','hymn','worship','chapel','religious','spirit'], \
	['contemporary','alternative','acoustic','garage','ecclectic','new age','alternative','alternative','alternative'], \
	['top','2000s', 'hits','variety','pop','90s','billboard','hot','top'], \
	['talk','comedy','public','politics','sports','news','educational','commun','college'], \
	['spanish','french','foreign','latin','celtic','caribbean','africa','world','asia']
	];

failed_urls = 0

for state_index in range(0,len(states)):

	state = states[state_index][1]

	try: 
	    response = urllib2.urlopen(str(base_url+state))
	except urllib2.HTTPError, e:
	    flog.write( "\n"+state+'_wiki_genres.txt:    FAILED\tHTTPError = ' + str(e.code)+"\n"+"\n")
	    continue
	except urllib2.URLError, e:
	    flog.write( "\n"+state+'_wiki_genres.txt:    FAILED\tURLError = ' + str(e.reason)+"\n"+"\n")
	    continue
	except httplib.HTTPException, e:
	    flog.write( "\n"+state+'_wiki_genres.txt:    FAILED\tHTTPException'+"\n")
	    continue

	html = response.read()
	soup = BeautifulSoup(html)

	fout = open(state+"_wiki_genres.txt","w")
	fout.write(soup.get_text().encode('utf-8'))
	fout.close()

	fin = open(state+"_wiki_genres.txt","r")
	lines = fin.readlines()

	flog.write( str(state+"_wiki_genres.txt:   ")+str(len(lines))+" lines "+"\n")

	#do nothing until you see this string
	trigger = 'The following is a list of FCC-licensed radio stations'

	#stop when you see this string
	halt = 'This list is complete'

	#read this many GARBAGE lines after the trigger
	offset = 9

	triggered = 0
	skip_lines = 0
	read_consecutively = 0
	line_num = -1
	wait_2blanks = 0

	callsigns = list()
	freqs = list()
	raw_genres = list()

	for line in lines:
		line_num = line_num+1

		if halt.lower() in line.lower():
			break

		if wait_2blanks:
			if not len(lines[line_num-2]) < 2 or not len(lines[line_num-1]) < 2:
				continue
			else:
				wait_2blanks = 0

		if len(lines) < line_num+10:
			flog.write("......REACHING END OF FILE.......\n")
			continue

		if skip_lines:
			skip_lines = skip_lines-1
			continue

		if triggered==0:
			if not trigger in line:
				continue
			else:
				skip_lines = offset
				triggered = 1
				continue

		#read out callsign, FM freq, nothing, nothing, genre
		
		temp_line = lines[line_num+1].split(' ')

		if len(temp_line) < 2 or halt.lower() in lines[line_num].lower() or halt.lower() in lines[line_num+1].lower():
			wait_2blanks = 1
			continue

		if (' AM' in lines[line_num+1]):
			#bail when these start becoming FM stations
			wait_2blanks = 1
			continue

		wait_2blanks = 1
		callsigns.append(line.strip('\n'))
		freqs.append(temp_line[0])

		temp_genre = lines[line_num+4]
		if "sale pending" in temp_genre:
			temp_genre = lines[line_num+5]

		if len(temp_genre) > 0:
			raw_genres.append(temp_genre.strip('\n'))
		else:
			raw_genres.append(" ")

		#for i in range(0, len(callsigns)):
		#	flog.write( callsigns[i]+'\t\t'+freqs[i]+'\t\t'+raw_genres[i]+"\n")

	flog.write( "\tidentified "+str(len(raw_genres))+" raw FM station genres"+"\n")

	#compose the actual list of genres
	calls_db = list()
	genres_db = list()
	freqs_db = list()
	states_db = list()

	for i in range(0,len(raw_genres)):

		if len(raw_genres[i]) < 2:
			continue

		bail = False

		for j in range(0,len(genre_keywords)):
			if bail:
				continue

			for k in range(0,len(genre_keywords[j])):
				if bail:
					continue

				#flog.write( genre_keywords[j][k]+"\n")

				if genre_keywords[j][k].lower() in raw_genres[i].lower():
					flog.write( "\t\t\t"+raw_genres[i]+" ----> "+genre_nicknames[j]+"\n")
					raw_genres[i] = genre_nicknames[j]
					bail = True

		if not bail:
			raw_genres[i] = re.sub(r'\W+', '', raw_genres[i])
			flog.write( "\t\t\t"+raw_genres[i]+" |||||||||| "+raw_genres[i]+"\n")

		#copy over the right information to calls_db, etc.
		genres_db.append(raw_genres[i])
		calls_db.append(callsigns[i])
		freqs_db.append(freqs[i])
		states_db.append(states[state_index][0])

	#update the database with these genres

	#connect to the database
	conn = psycopg2.connect("dbname=fmStations user=postgres")

	#create a cursor to perform database operations
	cur = conn.cursor()

	for i in range(0,len(genres_db)):
		#compose each update statement and execute
		statement = 'UPDATE stations'+" SET genre=\'"+genres_db[i]+"\' WHERE call=\'"+calls_db[i]+"\' AND freq=\'"+freqs_db[i]+'\' AND state=\''+states_db[i]+'\''
		#print(statement)
		flog.write( statement+"\n")
		cur.execute(statement)

	#make DB changes persistent
	conn.commit()

	#close database connection
	cur.close()
	conn.close()

	#remove the txt file for this state
	flog.write("removing file: "+str(state+"_wiki_genres.txt")+"\n")
	os.remove(str(state+"_wiki_genres.txt"))