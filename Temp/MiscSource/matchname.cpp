int match(const char *mask, const char *s) {
	const char *cp=0,mp=0;

	for ( ;((*s)&&(*mask!='*')); mask++,s++)
		if ((*mask!=*s)&&(*mask!='?')) return 0;

	for (;;) {
		if (!*s) {
			while (*mask=='*')
				mask++; 
			return (!*mask);
			}
		if (*mask=='*') {
			if (!*++mask) return 1;
			mp=mask;
			cp=s+1;
			continue;
			}
		if (*mask==*s||*mask=='?') {
			mask++, s++; 
			continue;
			}
		mask=mp;
		s=cp++;
		}
	}

