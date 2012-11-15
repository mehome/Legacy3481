#ifndef SELECTIMAGE_H
#define SELECTIMAGE_H

class selectimageclass {
	public:
		struct indeximage *indexhead,*indextail,*indexptr;
		struct imagelist *blocka,*blockb;
		static HBITMAP multiselectimage;

		selectimageclass();
		~selectimageclass();
		void selectimage(struct imagelist *imageptr);
		void shiftselect(struct imagelist *imageptr);
		void resetlist(BOOL unselect);
		void listselection();
	private:
	};

#endif /* SELECTIMAGE_H */
