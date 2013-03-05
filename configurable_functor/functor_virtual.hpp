class functor
{
public:
  virtual int operator()(void) = 0;
};

struct func1 : public functor
{
  virtual int operator()(void);
};

struct func2 : public functor
{
  virtual int operator()(void);
};
