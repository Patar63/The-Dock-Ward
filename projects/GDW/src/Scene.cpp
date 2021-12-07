#include "Scene.h"

SMI_Scene::SMI_Scene()
{
    //scene is active and not paused
	isActive = true;
	isPaused = false;

    //setting up physics world
    CollisionConfig = new btDefaultCollisionConfiguration(); //default collision config
    Dispatcher = new btCollisionDispatcher(CollisionConfig); //default collision dispatcher
    OverlappingPairCache = new btDbvtBroadphase();//basic board phase
    Solver = new btSequentialImpulseConstraintSolver;//default collision solver
    Collisions = std::vector<SMI_Collision::sptr>();

    //create the physics world
    physicsWorld = new btDiscreteDynamicsWorld(Dispatcher, OverlappingPairCache, Solver, CollisionConfig);
    physicsWorld->setGravity(btVector3(0.f, 0.f, 0.f));

    //create registry
    Store = entt::registry();
    camera = nullptr;
}

SMI_Scene::~SMI_Scene()
{
    //delete all the physics world stuff
        //delete the physics objects
    for (auto i = physicsWorld->getNumCollisionObjects() - 1; i >= 0; i--) {
        //get the object and it's rigid body
        btCollisionObject* PhyObject = physicsWorld->getCollisionObjectArray()[i];
        btRigidBody* PhysRigidBod = btRigidBody::upcast(PhyObject);
        //if it has a motion state, remove that
        if (PhysRigidBod != nullptr && PhysRigidBod->getMotionState() != nullptr) {
            delete PhysRigidBod->getMotionState();
        }
        //remove the object from the physics world
        physicsWorld->removeCollisionObject(PhyObject);
        //and delete it
        delete PhyObject;
    }

    //delete the physics world and it's attributes
    delete physicsWorld;
    delete Solver;
    delete OverlappingPairCache;
    delete Dispatcher;
    delete CollisionConfig;
}

entt::entity SMI_Scene::CreateEntity()
{
    return Store.create();
}

void SMI_Scene::DeleteEntity(entt::entity target)
{
    if (Store.has<SMI_Physics>(target))
    {
        btRigidBody* TargetBody = Store.get<SMI_Physics>(target).getRigidBody();
        delete TargetBody->getMotionState();
        delete TargetBody->getCollisionShape();
        physicsWorld->removeRigidBody(TargetBody);
        delete TargetBody;
    }

    Store.destroy(target);
}

void SMI_Scene::InitScene()
{
}

void SMI_Scene::Update(float deltaTime)
{
    if (!isPaused)
    {
        physicsWorld->stepSimulation(deltaTime);
        
        //creating view for physics
        auto PhysicsView = Store.view<SMI_Physics>();
        for (auto entity : PhysicsView)
        {
            SMI_Physics& phys = GetComponent<SMI_Physics>(entity);
            phys.getRigidBody()->setActivationState(true);

            //set gravity value
            if (phys.getHasGravity())
            {
                phys.getRigidBody()->setGravity(btVector3(gravity.x, gravity.y, gravity.z));
            }
            else
            {
                phys.getRigidBody()->setGravity(btVector3(0.f, 0.f, 0.f));
            }

            phys.Update(deltaTime);
        }

        CollisionManage();

        auto TransPhysView = Store.view<SMI_Physics, SMI_Transform>();
        for (auto entity : TransPhysView)
        {
            SMI_Transform& trans = GetComponent<SMI_Transform>(entity);
            SMI_Physics& phys = GetComponent<SMI_Physics>(entity);

            trans.setPos(phys.GetPosition());
        }
    }
}

void SMI_Scene::Render()
{
    auto RenderView = Store.view<Renderer, SMI_Transform>();
    for (auto entity : RenderView)
    {
        SMI_Transform& trans = GetComponent<SMI_Transform>(entity);
        Renderer& rend = GetComponent<Renderer>(entity);

        UniformMatrixObject<glm::mat4>::Sptr ModelMatrix = std::dynamic_pointer_cast<UniformMatrixObject<glm::mat4>>
                                                     (rend.getMaterial()->getUniform("Model"));
        UniformMatrixObject<glm::mat4>::Sptr MVPMatrix = std::dynamic_pointer_cast<UniformMatrixObject<glm::mat4>>
                                                   (rend.getMaterial()->getUniform("MVP"));

        //check if nullptr and create uniform if needed
        if (ModelMatrix == nullptr)
        {
            ModelMatrix = UniformMatrixObject<glm::mat4>::Create();
            ModelMatrix->setName("Model");
            ModelMatrix->setData(glm::mat4());
            rend.getMaterial()->setUniform(ModelMatrix);
        }
        if (MVPMatrix == nullptr)
        {
            MVPMatrix = UniformMatrixObject<glm::mat4>::Create();
            MVPMatrix->setName("MVP");
            MVPMatrix->setData(glm::mat4());
            rend.getMaterial()->setUniform(MVPMatrix);
        }

        ModelMatrix->setData(trans.getGlobal());
        if (camera != nullptr)
        {
            MVPMatrix->setData(camera->GetViewProjection() * trans.getGlobal());
        }

        rend.Render();
    }
}

void SMI_Scene::PostRender()
{
}

void SMI_Scene::CollisionManage()
{
    //based on code from https://andysomogyi.github.io/mechanica/bullet.html
    Collisions.clear();

    int manifolds = physicsWorld->getDispatcher()->getNumManifolds();
    for (int i = 0; i < manifolds; i++)
    {
        btPersistentManifold* Contact = physicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
        //collision body 1 and 2
        const btCollisionObject* obj1 = Contact->getBody0();
        const btCollisionObject* obj2 = Contact->getBody1();

        //int for number of contacts
        int numcon = Contact->getNumContacts();
        for (int j = 0; j < numcon; j++)
        {
            btManifoldPoint& point = Contact->getContactPoint(j);
            if (point.getDistance() < 0.f)
            {
                const btRigidBody* FirstBody = btRigidBody::upcast(obj1);
                const btRigidBody* SecondBody = btRigidBody::upcast(obj2);

                SMI_Collision::sptr NewCollide = SMI_Collision::Create();
                NewCollide->setB1(static_cast<entt::entity>(reinterpret_cast<uint32_t>(FirstBody->getUserPointer())));
                NewCollide->setB2(static_cast<entt::entity>(reinterpret_cast<uint32_t>(SecondBody->getUserPointer())));

                bool Add = true;
                for (int k = 0; k < Collisions.size(); k++)
                {
                    if (SMI_Collision::Same(NewCollide, Collisions[k]))
                    {
                        Add = false;
                        break;
                    }
                }

                if (Add)
                {
                    Collisions.push_back(NewCollide);
                }
            }
        }
    }
}
