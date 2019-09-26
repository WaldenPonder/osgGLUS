#include "CollectLinesVisitor.h"
#include <osg\Geometry>

class GetVertexArrayVisitor : public osg::ArrayVisitor
{
 public:
	GetVertexArrayVisitor()
	{
	}

	virtual void apply(osg::Vec3Array& array)
	{
		for (int i = 0; i < array.size(); i++)
		{
			pts_.push_back(array.asVector()[i]);
		}
	}

	virtual void apply(osg::Vec4Array& array)
	{
		for (int i = 0; i < array.size(); i++)
		{
			osg::Vec4 v = array.asVector()[i];
			pts_.push_back(osg::Vec3(v.x(), v.y(), v.z()));
		}
	}

	virtual void apply(osg::Vec3dArray& array)
	{
		for (int i = 0; i < array.size(); i++)
		{
			pts_.push_back(array.asVector()[i]);
		}
	}

	virtual void apply(osg::Vec4dArray& array)
	{
		for (int i = 0; i < array.size(); i++)
		{
			osg::Vec4 v = array.asVector()[i];
			pts_.push_back(osg::Vec3(v.x(), v.y(), v.z()));
		}
	}

	std::vector<osg::Vec3> pts_;

 protected:
	GetVertexArrayVisitor& operator=(const GetVertexArrayVisitor&) { return *this; }
};

class LineIndexFunctor : public osg::PrimitiveIndexFunctor
{
 public:
	LineIndexFunctor(const std::vector<osg::Vec3>& pts, std::vector<CollectLinesVisitor::Line>& lines)
		: pts_(pts), lines_(lines) {}

	virtual void setVertexArray(unsigned int, const osg::Vec2*) {}
	virtual void setVertexArray(unsigned int, const osg::Vec3*) {}
	virtual void setVertexArray(unsigned int, const osg::Vec4*) {}
	virtual void setVertexArray(unsigned int, const osg::Vec2d*) {}
	virtual void setVertexArray(unsigned int, const osg::Vec3d*) {}
	virtual void setVertexArray(unsigned int, const osg::Vec4d*) {}

	virtual void drawArrays(GLenum mode, GLint first, GLsizei count)
	{
		switch (mode)
		{
		case (GL_LINES):
		{
			unsigned int pos = first + 1;
			for (GLsizei i = 0; i < count; i += 2, pos += 2)
			{
				if (pos < pts_.size())
					lines_.push_back(CollectLinesVisitor::Line{ pts_[pos], pts_[pos - 1] });
			}
			break;
		}
		case (GL_LINE_STRIP):
		{
			unsigned int pos = first + 1;
			for (GLsizei i = 0; i < count; i += 1, pos += 1)
			{
				if (pos < pts_.size())
					lines_.push_back(CollectLinesVisitor::Line{ pts_[pos], pts_[pos - 1] });
			}

			break;
		}
		case (GL_LINE_LOOP):
		{
			unsigned int pos = first + 1;
			for (GLsizei i = 0; i < count; i += 1, pos += 1)
			{
				if (pos < pts_.size())
					lines_.push_back(CollectLinesVisitor::Line{ pts_[pos], pts_[pos - 1] });
			}

			if (first < pts_.size() && first + count - 1 < pts_.size())
				lines_.push_back(CollectLinesVisitor::Line{ pts_[first], pts_[first + count - 1] });
			break;
		}
		}
	}

	virtual void drawElements(GLenum mode, GLsizei count, const GLubyte* indices)
	{
		if (indices == 0 || count == 0) return;

		typedef GLubyte		 Index;
		typedef const Index* IndexPointer;

		switch (mode)
		{
		case (GL_LINES):
		{
			IndexPointer iptr = indices;
			for (GLsizei i = 0; i < count; i += 2)
			{
				size_t s = iptr[i], e = iptr[i + 1];
				if (s < pts_.size() && e < pts_.size())
					lines_.push_back(CollectLinesVisitor::Line{ pts_[s], pts_[e] });
			}
			break;
		}
		case (GL_LINE_STRIP):
		{
			IndexPointer iptr = indices;
			for (GLsizei i = 0; i < count; i += 1)
			{
				size_t s = iptr[i], e = iptr[i + 1];
				if (s < pts_.size() && e < pts_.size())
					lines_.push_back(CollectLinesVisitor::Line{ pts_[s], pts_[e] });
			}
			break;
		}
		case (GL_LINE_LOOP):
		{
			IndexPointer iptr = indices;

			for (GLsizei i = 0; i < count; i += 1)
			{
				size_t s = iptr[i], e = iptr[i + 1];

				if (s < pts_.size() && e < pts_.size())
					lines_.push_back(CollectLinesVisitor::Line{ pts_[s], pts_[e] });
			}
			size_t s = iptr[0], e = iptr[count - 1];
			lines_.push_back(CollectLinesVisitor::Line{ pts_[s], pts_[e] });
			break;
		}
		}
	}

	virtual void drawElements(GLenum mode, GLsizei count, const GLushort* indices)
	{
		if (indices == 0 || count == 0) return;

		typedef GLushort	 Index;
		typedef const Index* IndexPointer;

		switch (mode)
		{
		case (GL_LINES):
		{
			IndexPointer iptr = indices;
			for (GLsizei i = 0; i < count; i += 2)
			{
				size_t s = iptr[i], e = iptr[i + 1];
				if (s < pts_.size() && e < pts_.size())
					lines_.push_back(CollectLinesVisitor::Line{ pts_[s], pts_[e] });
			}
			break;
		}
		case (GL_LINE_STRIP):
		{
			IndexPointer iptr = indices;
			for (GLsizei i = 0; i < count; i += 1)
			{
				size_t s = iptr[i], e = iptr[i + 1];
				if (s < pts_.size() && e < pts_.size())
					lines_.push_back(CollectLinesVisitor::Line{ pts_[s], pts_[e] });
			}
			break;
		}
		case (GL_LINE_LOOP):
		{
			IndexPointer iptr = indices;

			for (GLsizei i = 0; i < count; i += 1)
			{
				size_t s = iptr[i], e = iptr[i + 1];

				if (s < pts_.size() && e < pts_.size())
					lines_.push_back(CollectLinesVisitor::Line{ pts_[s], pts_[e] });
			}
			size_t s = iptr[0], e = iptr[count - 1];
			lines_.push_back(CollectLinesVisitor::Line{ pts_[s], pts_[e] });
			break;
		}
		}
	}

	virtual void drawElements(GLenum mode, GLsizei count, const GLuint* indices)
	{
		if (indices == 0 || count == 0) return;

		typedef GLuint		 Index;
		typedef const Index* IndexPointer;

		switch (mode)
		{
		case (GL_LINES):
		{
			IndexPointer iptr = indices;
			for (GLsizei i = 0; i < count; i += 2)
			{
				size_t s = iptr[i], e = iptr[i + 1];
				if (s < pts_.size() && e < pts_.size())
				lines_.push_back(CollectLinesVisitor::Line{ pts_[s], pts_[e] });
			}
			break;
		}
		case (GL_LINE_STRIP):
		{
			IndexPointer iptr = indices;
			for (GLsizei i = 0; i < count; i += 1)
			{
				size_t s = iptr[i], e = iptr[i + 1];
				if (s < pts_.size() && e < pts_.size())
				lines_.push_back(CollectLinesVisitor::Line{ pts_[s], pts_[e] });
			}
			break;
		}
		case (GL_LINE_LOOP):
		{
			IndexPointer iptr = indices;

			for (GLsizei i = 0; i < count; i += 1)
			{
				size_t s = iptr[i], e = iptr[i + 1];

				if(s < pts_.size() && e < pts_.size())
				lines_.push_back(CollectLinesVisitor::Line{ pts_[s], pts_[e] });
			}
			size_t s = iptr[0], e = iptr[count - 1];
			lines_.push_back(CollectLinesVisitor::Line{ pts_[s], pts_[e] });
			break;
		}
		}
	}

	const std::vector<osg::Vec3>&			pts_;
	std::vector<CollectLinesVisitor::Line>& lines_;
};

CollectLinesVisitor::CollectLinesVisitor(bool bWorldSpace /*= true*/) : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN), bWorldSpace_(bWorldSpace)
{
}

CollectLinesVisitor::~CollectLinesVisitor()
{
}

void CollectLinesVisitor::apply(osg::Geometry& geometry)
{
	GetVertexArrayVisitor gva;
	geometry.getVertexArray()->accept(gva);

	LineIndexFunctor indexFunc(gva.pts_, lines_);
	geometry.accept(indexFunc);

	osg::Vec3 p1, p2;
	std::make_pair(p1, p2);
}
