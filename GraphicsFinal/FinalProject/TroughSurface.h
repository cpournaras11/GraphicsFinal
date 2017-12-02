#ifndef TROUGH_SURFACE_H
#define TROUGH_SURFACE_H

/**
* Trough surface geometry node.
*/
class TroughSurface : public TriSurface
{
public:
    /**
    * Creates a trough (half cylinder) with specified number of sides (how many times the top/bottom circles
    * are divided) and number of stacks (subdivisions in z). z values range
    * from -0.25 to 0.25 (unit height). The radius is 1 for the top and bottom. End caps are not included!
    * @param  numSides   Number of sides (divisions of the top/bottom)
    * @param  numStacks  Number of height divisions
    */
    TroughSurface(uint32_t numSides,
        uint32_t numStacks,
        int posLoc,
        int normLoc)
    {
        /*
        * Create the vertex list and normals
        */
        float z = -0.25f;
        float zDelta = 0.5f / numStacks;

        float thetaDelta = (2.0f * kPi) / static_cast<float>(numSides);

        bool flipNormals = false;

        VertexAndNormal vtx;

        // Iterate over the exterior vertices first, then the interior vertices
        for (uint32_t i = 0; i < 2; i++)
        {
            // Iterate over the stacks (rows)
            for (uint32_t j = 0; j <= numStacks; j++)
            {
                float theta = 0.0f;

                // Iterate over the sides (columns)
                for (uint32_t k = 0; k < numSides; k++)
                {
                    // Add the vertex position and normal at the current angle
                    float cosTheta = cosf(theta);
                    float sinTheta = sinf(theta);

                    vtx.vertex.Set(cosTheta, sinTheta, z);

                    vtx.normal.x = ((flipNormals == true) ? -cosTheta : cosTheta);
                    vtx.normal.y = ((flipNormals == true) ? -sinTheta : sinTheta);

                    vertices.push_back(vtx);

                    // Increment the angle
                    theta += thetaDelta;
                }

                // Add the vertex position and normal at theta = 0
                vtx.vertex.Set(1.0f, 0.0f, z);

                vtx.normal.x = ((flipNormals == true) ? -1.0f : 1.0f);
                vtx.normal.y = 0.0f;

                vertices.push_back(vtx);

                // Increment the height
                z += zDelta;
            }

            // Set the height and delta so that we traverse down for the interior vertices
            z = 0.25f;
            zDelta = -zDelta;

            flipNormals = true;
        }

        /*
        * Construct the face list for a triangle strip
        */
        uint32_t numRows = (numStacks * 2) + 1;
        uint32_t numCols = numSides + 1;

        uint32_t col = 0;

        // Iterate over the rows
        for (uint32_t row = 0; row < numRows; row++)
        {
            // Add the first vertex in this row to the degenerate triangle
            if (row > 0)
            {
                faces.push_back(GetIndex(row + 1, 0, numCols));
            }

            // Iterate over the sides (columns)
            for (col = 0; col < numCols; col++)
            {
                faces.push_back(GetIndex(row + 1, col, numCols));
                faces.push_back(GetIndex(row, col, numCols));
            }

            faces.push_back(GetIndex(row, col, numCols));
        }

        // Construct the face list and create VBOs
        //ConstructRowColFaceList(numRows * 2, numCols);

        CreateVertexBuffers(posLoc, normLoc);
    }

    /**
    * Draw this geometry node.
    */
    virtual void Draw(SceneState & scene_state)
    {
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLE_STRIP, (GLsizei)face_count, GL_UNSIGNED_SHORT, (void *)0);
        glBindVertexArray(0);
    }

private:
    // Make default constructor private to force use of the constructor
    // with number of subdivisions.
    TroughSurface() { };
};

class TexturedTroughSurface : public TexturedTriSurface
{
public:
		/**
		* Creates a trough (half cylinder) with specified number of sides (how many times the top/bottom circles
		* are divided) and number of stacks (subdivisions in z). z values range
		* from -0.25 to 0.25 (unit height). The radius is 1 for the top and bottom. End caps are not included!
		* @param  numSides   Number of sides (divisions of the top/bottom)
		* @param  numStacks  Number of height divisions
		*/
		TexturedTroughSurface(uint32_t numSides,
				uint32_t numStacks,
				int posLoc,
				int normLoc,                       
				const int texture_loc,
				const int tangent_loc,
				const int bitangent_loc)
		{
				/*
				* Create the vertex list and normals
				*/
				float z = -0.25f;
				float zDelta = 0.5f / numStacks;

				float thetaDelta = (2.0f * kPi) / static_cast<float>(numSides);

				bool flipNormals = false;

				PNTVertex vtx;

				// Iterate over the exterior vertices first, then the interior vertices
				for (uint32_t i = 0; i < 2; i++)
				{
						// Iterate over the stacks (rows)
						for (uint32_t j = 0; j <= numStacks; j++)
						{
								float theta = 0.0f;
								vtx.t = z*2 + 0.5f;

								printf("%f \n", vtx.s);

								// Iterate over the sides (columns)
								for (uint32_t k = 0; k < numSides; k++)
								{
										// Add the vertex position and normal at the current angle
										float cosTheta = cosf(theta);
										float sinTheta = sinf(theta);

										vtx.s = (k / float(numSides));
										if (vtx.s > .974)
												vtx.s = .974;
										if (vtx.s < 0.025)
												vtx.s = .026;
										vtx.vertex.Set(cosTheta, sinTheta, z);

										vtx.normal.x = ((flipNormals == true) ? -cosTheta : cosTheta);
										vtx.normal.y = ((flipNormals == true) ? -sinTheta : sinTheta);

										vertices.push_back(vtx);

										// Increment the angle
										theta += thetaDelta;
								}

								// Add the vertex position and normal at theta = 0
								vtx.vertex.Set(1.0f, 0.0f, z);

								vtx.normal.x = ((flipNormals == true) ? -1.0f : 1.0f);
								vtx.normal.y = 0.0f;

								vertices.push_back(vtx);

								// Increment the height
								z += zDelta;

						}

						// Set the height and delta so that we traverse down for the interior vertices
						z = 0.25f;
						zDelta = -zDelta;

						flipNormals = true;
				}

				/*
				* Construct the face list for a triangle strip
				*/
				uint32_t numRows = (numStacks * 2) + 1;
				uint32_t numCols = numSides + 1;

				uint32_t col = 0;

				// Iterate over the rows
				for (uint32_t row = 0; row < numRows; row++)
				{
						// Add the first vertex in this row to the degenerate triangle
						if (row > 0)
						{
								faces.push_back(GetIndex(row + 1, 0, numCols));
						}

						// Iterate over the sides (columns)
						for (col = 0; col < numCols; col++)
						{
								faces.push_back(GetIndex(row + 1, col, numCols));
								faces.push_back(GetIndex(row, col, numCols));
						}

						faces.push_back(GetIndex(row, col, numCols));
				}

				// Construct the face list and create VBOs
				//ConstructRowColFaceList(numRows * 2, numCols);

				CreateVertexBuffers(posLoc, normLoc,texture_loc, tangent_loc, bitangent_loc);
		}

		/**
		* Draw this geometry node.
		*/
		virtual void Draw(SceneState & scene_state)
		{
				glBindVertexArray(vao);
				glDrawElements(GL_TRIANGLE_STRIP, (GLsizei)face_count, GL_UNSIGNED_SHORT, (void *)0);
				glBindVertexArray(0);
		}

private:
		// Make default constructor private to force use of the constructor
		// with number of subdivisions.
		TexturedTroughSurface() { };
};

#endif