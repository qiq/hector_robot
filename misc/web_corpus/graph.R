# Czech, slovak, other, unaccented

#		clanky			diskuse			tvorivost		overall
# cs		452585914	99.07+	1143073782	72.38+	983346836	83.63	2579006532	80.29+
# sk		3067468		0.67	51297087	3.24	107604069	9.15	161968624	5.04
# en		501500		0.11	1488875		0.09	10962822	0.93	12953197	0.4
# other		64281		0.01	4616883		0.29	13067099	1.11	17748263	0.55
# acc		456219163	0.13	1200476627	23.99	1114980826	5.18	2771676616	13.71
# total(dedup)	456830973	100	1579301675	100	1175831877	100	3211964525	100
# (+ means add 0.01)

# total(tok)	516703278	1674143836	1574181258		3765028372

#dev.off()

w <- read.csv("graph.data", sep="\t", header=T)

# Expand right side of clipping rect to make room for the legend
par(xpd=T, mar=par()$mar+c(0,0,0,8))

#mycol <- c(rgb(255,106,0,max=255), rgb(255,164,0,max=255), rgb(0,160,138,max=255), rgb(93,208,192,max=255), rgb(16,73,169,max=255))
mycol <- rev(heat.colors(5))

barplot(as.matrix(w), ylab="Percentage", col=mycol, space=0.1, cex.axis=1.2, cex.names=1.2, las=1, names.arg=c("News", "Discussions", "Blogs", "Overall"), cex.lab=1.2)

# Graph autos (transposing the matrix) using heat colors,  
# put 10% of the space between each bar, and make labels  
# smaller with horizontal y-axis labels
#barplot(t(autos_data), main="Autos", ylab="Total", 
#   col=heat.colors(3), space=0.1, cex.axis=0.8, las=1,
#   names.arg=c("Mon","Tue","Wed","Thu","Fri"), cex=0.8) 
   
# Place the legend at (6,30) using heat colors
legend(4.5, 60, c("Other languages", "English", "Slovak", "Unaccented", "Czech"), cex=1.2, fill=rev(mycol), bty="n");

dev.copy2eps(file="graph.eps")
#dev.off()

# Restore default clipping rect
#par(mar=c(5, 4, 4, 2) + 0.1)
